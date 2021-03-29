#include <stdbool.h>
#include <signal.h>
#include <sys/time.h>

#include "dds_dcps.h"
#include "consensus/replica.h"
#include "DDSCreator/CheckStatus.h"
#include "DDSEntitiesManager.h"
#include "datamodel.h"
#include "evaluation/evaluator.h"

replica_t *this_replica;

void perform_voting(RevPiDDS_Input* input, const replica_result_t* results, const size_t length) {

    DDS_ReturnCode_t status;

    printf("Got new voting material\n");

    if (this_replica->role != LEADER) {
        printf("Got new voting material but not leader anymore\n");
        return;
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

        DDS_InstanceHandle_t handle = DDS_Entity_get_instance_handle(input);
        status = RevPiDDS_InputDataWriter_dispose(
            input_DataWriter,
            input,
            handle);
        checkStatus(status, "Input_DataWriter Dispose input");
    }

}

void on_no_results(void) {
    printf("Failed to deliver!!!!!\n");
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: %s [replicaID]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    initialize_evaluator();

    uint8_t replica_ID = (uint8_t)atoi(argv[1]);

    DDS_ReturnCode_t status;
    unsigned long i = 0;
    DDS_sequence_RevPiDDS_Input* message_seq = DDS_sequence_RevPiDDS_Input__alloc();
    DDS_SampleInfoSeq* message_infoSeq = DDS_SampleInfoSeq__alloc();
    DDS_Duration_t input_Timeout = DDS_DURATION_INFINITE;
    bool running = true;

    DDSSetup();
    // uint8_t message = 0;

    initialize_replica(replica_ID);

    status = DDS_WaitSet_attach_condition(input_WaitSet, input_ReadCondition);
    checkStatus(status, "DDS_WaitSet_attach_condition (input_ReadCondition)");

    while (running) {
        status = DDS_WaitSet_wait(input_WaitSet, input_GuardList, &input_Timeout);
        pthread_mutex_lock(&this_replica->consensus_mutex);
        if (this_replica->role != LEADER) {
            pthread_mutex_unlock(&this_replica->consensus_mutex);
            continue;
        }
        printf("Input waitSet triggered\n");
        if (status == DDS_RETCODE_OK) {

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

                        if (message_seq->_buffer[i].id == 0) {
                            running = false;
                            break;
                        }

                        pthread_mutex_unlock(&this_replica->consensus_mutex);
                        cluster_process(&message_seq->_buffer[i], &perform_voting, &on_no_results);
                        pthread_mutex_lock(&this_replica->consensus_mutex);
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

        } else {
            pthread_mutex_unlock(&this_replica->consensus_mutex);
            checkStatus(status, "DDS_WaitSet_wait (Input Waitset)");
        }
    }

    status = DDS_WaitSet_detach_condition(input_WaitSet, input_ReadCondition);
    checkStatus(status, "DDS_WaitSet_detach_condition");


    status = RevPiDDS_InputDataReader_delete_readcondition(input_DataReader, input_ReadCondition);
    checkStatus(status, "RevPiDDS_InputDataReader_delete_readcondition");


    teardown_replica();
    DDSCleanup();
    DDS_free(message_seq);
    DDS_free(message_infoSeq);
}