#include <stdbool.h>
#include <signal.h>
#include <sys/time.h>

#include "dds_dcps.h"
#include "consensus/replica.h"
#include "DDSCreator/CheckStatus.h"
#include "DDSEntitiesManager.h"
#include "state/movementAuthority.h"
#include "state/balise.h"
#include "state/train.h"
#include "datamodel.h"
#include "evaluation/evaluator.h"
#include "consensus/decisionMaking.h"

#define STOP_TRAIN_ID 0
#define MOVEMENT_AUTHORITY_RBC_ID 1
#define BALISE_LINKING_RBC_ID 2

replica_t *this_replica;

void perform_voting(RevPiDDS_Input* balise_telegram, const replica_result_t* results, const size_t length) {

    DDS_ReturnCode_t status;

    printf("Got new voting material\n");

    if (length > 0) {
        if (results[0].should_break) {
            train_state_t train_state;
            bool has_state = get_train_state(&train_state);
            if (has_state) {
                evaluator_train_stopped(train_state.position.position, results[0].reason);
            }
        }
    }

    if (this_replica->role != LEADER) {
        printf("Got new voting material but not leader anymore\n");
        return;
    }

    if (balise_telegram != NULL && !results->should_break) {

        balise_t referenced_balise;
        bool is_linked = get_balise_if_linked(balise_telegram->data._buffer[2], &referenced_balise);
        if (is_linked) {
            set_train_position(referenced_balise.position);
        }
    }

    // TODO Can it happen that replica died while processing and come back alive when result was processed by another leader?
    // Prevent the result from being processed (committed) twice
    for (size_t i = 0; i < length; ++i) {
        replica_result_t result = results[i];
        printf("Reply from Replica: %d\n", result.replica_id);

        if (result.term > this_replica->current_term) {
            printf("The reply was made by a follower from the future\n");
            continue;
        }

        if (balise_telegram != NULL) {
            DDS_InstanceHandle_t handle = DDS_Entity_get_instance_handle(balise_telegram);
            status = RevPiDDS_InputDataWriter_dispose(
                input_DataWriter,
                balise_telegram,
                handle);
            checkStatus(status, "Input_DataWriter Dispose input");
        }

    }

}

void on_no_results(void) {
    printf("Failed to deliver!!!!!\n");
}

void main_loop() {
    bool running = true;
    DDS_ReturnCode_t status;
    unsigned long i = 0;
    DDS_sequence_RevPiDDS_Input* message_seq = DDS_sequence_RevPiDDS_Input__alloc();
    DDS_SampleInfoSeq* message_infoSeq = DDS_SampleInfoSeq__alloc();
    DDS_Duration_t input_Timeout = {1 , 0};

    while (running) {
        status = DDS_WaitSet_wait(input_WaitSet, input_GuardList, &input_Timeout);
        pthread_mutex_lock(&this_replica->consensus_mutex);
        if (this_replica->role != LEADER) {
            pthread_mutex_unlock(&this_replica->consensus_mutex);
            continue;
        }
        if (status == DDS_RETCODE_OK) {
            printf("Input waitSet triggered\n");

            status = RevPiDDS_InputDataReader_take_w_condition(
                    input_DataReader,
                    message_seq,
                    message_infoSeq,
                    DDS_LENGTH_UNLIMITED,
                    input_ReadCondition);
            checkStatus(status, "RevPiDDS_InputDataReader_read_w_condition");

            if (message_seq->_length > 0) {
                for( i = 0; i < message_seq->_length ; i++ ) {
                    printf("\n    --- New message received ---");
                    if( message_infoSeq->_buffer[i].valid_data == TRUE ) {
                        printf("\n    Message : \"%d\"\n", message_seq->_buffer[i].id);

                        DDS_sequence_long data = message_seq->_buffer[i].data;
                        if (data._buffer[0] == STOP_TRAIN_ID) {
                            DDS_free(message_seq);
                            DDS_free(message_infoSeq);
                            return;
                        } else if (data._buffer[0] == MOVEMENT_AUTHORITY_RBC_ID) {
                            if(!parse_and_set_movement_authority(data)) {
                                printf("Error while parsing and setting MA data\n");
                            } else {
                                evaluator_start_new_jouney();
                            }
                        } else if (data._buffer[0] == BALISE_LINKING_RBC_ID) {
                            if (!set_linked_balises(data)) {
                                printf("Error setting linked balises\n");
                            }
                        } else {
                            pthread_mutex_unlock(&this_replica->consensus_mutex);
                            cluster_process(&message_seq->_buffer[i], &perform_voting, &on_no_results);
                            pthread_mutex_lock(&this_replica->consensus_mutex);
                        }
                    } else {
                        printf("\n    Data is invalid!\n");
                    }
                }
            } else {
                printf("Got an empty message\n");
            }

            pthread_mutex_unlock(&this_replica->consensus_mutex);
            fflush(stdout);

            status = RevPiDDS_InputDataReader_return_loan(input_DataReader, message_seq, message_infoSeq);
            checkStatus(status, "RevPiDDS_InputDataReader_return_loan");

        } else if (status == DDS_RETCODE_TIMEOUT) {
            printf("Calculate braking curve\n");
            // Emtpy input means only observe speed and braking curve
            pthread_mutex_unlock(&this_replica->consensus_mutex);
            cluster_process(NULL, &perform_voting, &on_no_results);
        } else {
            pthread_mutex_unlock(&this_replica->consensus_mutex);
            checkStatus(status, "DDS_WaitSet_wait (Input Waitset)");
        }
    }

    DDS_free(message_seq);
    DDS_free(message_infoSeq);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: %s [replicaID]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    initialize_evaluator();

    uint8_t replica_ID = (uint8_t)atoi(argv[1]);

    DDS_ReturnCode_t status;

    DDSSetup();
    // uint8_t message = 0;

    initialize_replica(replica_ID);

    status = DDS_WaitSet_attach_condition(input_WaitSet, input_ReadCondition);
    checkStatus(status, "DDS_WaitSet_attach_condition (input_ReadCondition)");

    main_loop();

    status = DDS_WaitSet_detach_condition(input_WaitSet, input_ReadCondition);
    checkStatus(status, "DDS_WaitSet_detach_condition");


    status = RevPiDDS_InputDataReader_delete_readcondition(input_DataReader, input_ReadCondition);
    checkStatus(status, "RevPiDDS_InputDataReader_delete_readcondition");


    teardown_replica();
    DDSCleanup();
}