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

#define STOP_TRAIN_ID 0
#define MOVEMENT_AUTHORITY_RBC_ID 1
#define BALISE_LINKING_RBC_ID 2

replica_t *this_replica;

void take_disposed_inputs() {

    DDS_ReturnCode_t status;
    DDS_sequence_RevPiDDS_Input msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};

    status = RevPiDDS_InputDataReader_take(
        input_DataReader,
        &msgSeq,
        &infoSeq,
        DDS_LENGTH_UNLIMITED,
        DDS_NOT_READ_SAMPLE_STATE | DDS_READ_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE | DDS_NOT_NEW_VIEW_STATE,
        DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE
    );
    checkStatus(status, "RevPiDDS_InputDataReader_take_instance (take_input_instance)");

}

void dispose_input(RevPiDDS_Input *input_message) {

    DDS_ReturnCode_t status;

    DDS_InstanceHandle_t handle = RevPiDDS_InputDataWriter_lookup_instance(
        input_DataWriter,
        input_message
    );
    checkHandle(&handle, "RevPiDDS_InputDataWriter_lookup_instance (dispose_input)");

    status = RevPiDDS_InputDataWriter_dispose(
        input_DataWriter,
        input_message,
        handle);
    checkStatus(status, "Input_DataWriter Dispose input");

}

bool vote_should_brake(const replica_result_t* results, const size_t length) {
    size_t i, j, count;;
    size_t most = 0;
    bool temp, elem = false;

    for (i = 0; i < length; i++) {
        if (results[i].term > this_replica->current_term) {
            printf("The reply was made by a follower from the future\n");
            continue;
        }
        temp = results[i].should_break;
        count = 1;
        for (j = i + 1; j < length; j++) {
            if (results[j].term > this_replica->current_term) {
                printf("The reply was made by a follower from the future\n");
                continue;
            }
            if (results[j].should_break == temp) {
                count++;
            }
        }

        if (most < count) {
            most = count;
            elem = results[i].should_break;
        }
    }

    return elem;
}

uint8_t vote_reason(const replica_result_t* results, const size_t length) {
    size_t i, j, count;;
    size_t most = 0;
    uint8_t temp, elem = 0;

    for (i = 0; i < length; i++) {
        if (results[i].term > this_replica->current_term) {
            printf("The reply was made by a follower from the future\n");
            continue;
        }
        temp = results[i].reason;
        printf("Vote a reason is: %d\n", temp);
        count = 1;
        for (j = i + 1; j < length; j++) {
            if (results[j].term > this_replica->current_term) {
                printf("The reply was made by a follower from the future\n");
                continue;
            }
            if (results[j].reason == temp) {
                count++;
            }
        }

        if (most < count) {
            most = count;
            elem = results[i].reason;
        }
    }

    return elem;
}

void perform_voting(const uint32_t inputID, const int baliseID, const replica_result_t* results, const size_t length) {

    replica_result_t final_result;
    train_state_t train_state;
    bool has_state = false;

    if (this_replica->role != LEADER) {
        printf("Got new voting material but not leader anymore\n");
        return;
    }

    printf("Got new voting material\n");

    final_result.should_break = vote_should_brake(results, length);
    final_result.reason = vote_reason(results, length);
    has_state = get_train_state(&train_state);

    if (final_result.should_break) {
        if (has_state) {
            printf("TRAIN SHOULD STOP BECAUSE OF: %d\n", final_result.reason);
            evaluator_train_stopped(train_state.position.max_position, baliseID, results[0].reason);
            set_train_driving(false);
        }
    }

    if (inputID != NO_ENTRIES_ID && !final_result.should_break) {
        printf("The referenced balise has ID: %d\n", baliseID);

        balise_t referenced_balise;
        bool is_linked = get_balise_if_linked(baliseID, &referenced_balise);
        if (is_linked) {
            printf("Setting train position to %d of balise %d\n", referenced_balise.position, referenced_balise.ID);
            if (has_state) {
                evaluator_reached_balise(train_state.position.position, baliseID);
            }
            set_train_position(referenced_balise.position);
        } 
    }

    if (inputID != NO_ENTRIES_ID) {
        RevPiDDS_Input *input_data;
        input_data = RevPiDDS_Input__alloc();
        input_data->id = inputID;
        dispose_input(input_data);

        DDS_free(input_data);
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
        if (status == DDS_RETCODE_OK) {

            take_disposed_inputs();

            status = RevPiDDS_InputDataReader_read_w_condition(
                    input_DataReader,
                    message_seq,
                    message_infoSeq,
                    DDS_LENGTH_UNLIMITED,
                    input_ReadCondition);
            checkStatus(status, "RevPiDDS_InputDataReader_read_w_condition");

            if (this_replica->role != LEADER) {

                pthread_mutex_unlock(&this_replica->consensus_mutex);
                status = RevPiDDS_InputDataReader_return_loan(input_DataReader, message_seq, message_infoSeq);
                checkStatus(status, "RevPiDDS_InputDataReader_return_loan");
                continue;
            }

            printf("Input waitSet triggered\n");

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

                            dispose_input(&message_seq->_buffer[i]);
                        } else if (data._buffer[0] == BALISE_LINKING_RBC_ID) {
                            if (!set_linked_balises(data)) {
                                printf("Error setting linked balises\n");
                            }
                            dispose_input(&message_seq->_buffer[i]);
                        } else {
                            pthread_mutex_unlock(&this_replica->consensus_mutex);
                            cluster_process(message_seq->_buffer[i].id, data._buffer[2], &perform_voting, &on_no_results);
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
            if (this_replica->role != LEADER) {
                pthread_mutex_unlock(&this_replica->consensus_mutex);
                continue;
            }
            printf("Calculate braking curve\n");
            // Emtpy input means only observe speed and braking curve
            pthread_mutex_unlock(&this_replica->consensus_mutex);
            cluster_process(NO_ENTRIES_ID, -1, &perform_voting, &on_no_results);
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

    // initialize_evaluator();

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