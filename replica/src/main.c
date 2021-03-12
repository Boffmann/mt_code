#include <stdbool.h>
#include <signal.h>
#include <sys/time.h>

#include "dds_dcps.h"
#include "consensus/replica.h"
#include "DDSCreator/CheckStatus.h"
#include "DDSEntitiesManager.h"
#include "datamodel.h"

replica_t *this_replica;

void perform_voting(const replica_result_t* results, const size_t length) {
    printf("Got new voting material\n");

    for (size_t i = 0; i < length; ++i) {
        printf("Reply from Replica: %d\n", results->replica_id);
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

    uint8_t replica_ID = (uint8_t)atoi(argv[1]);

    DDS_ReturnCode_t status;
    unsigned long i = 0;
    DDS_sequence_RevPiDDS_Input* message_seq = DDS_sequence_RevPiDDS_Input__alloc();
    DDS_SampleInfoSeq* message_infoSeq = DDS_SampleInfoSeq__alloc();
    DDS_Duration_t input_Timeout = DDS_DURATION_INFINITE;

    DDSSetup();

    initialize_replica(replica_ID);

    status = DDS_WaitSet_attach_condition(input_WaitSet, input_ReadCondition);
    checkStatus(status, "DDS_WaitSet_attach_condition (input_ReadCondition)");

    while (true) {
        status = DDS_WaitSet_wait(input_WaitSet, input_GuardList, &input_Timeout);

        if (status == DDS_RETCODE_OK) {
            if (this_replica->role != LEADER) {
                continue;
            }
            pthread_mutex_lock(&this_replica->consensus_mutex);

            status = RevPiDDS_InputDataReader_read_w_condition(input_DataReader, message_seq, message_infoSeq, DDS_LENGTH_UNLIMITED, input_ReadCondition);
            checkStatus(status, "RevPiDDS_InputDataReader_read_w_condition");

            for( i = 0; i < message_seq->_length ; i++ ) {
                printf("\n    --- New message received ---");
                if( message_infoSeq->_buffer[i].valid_data == TRUE ) {
                    printf("\n    Message : \"%d\"", message_seq->_buffer[i].test);
                } else {
                    printf("\n    Data is invalid!");
                }
            }
            fflush(stdout);
            log_entry_t new_entry;
            new_entry.id = 42;
            pthread_mutex_unlock(&this_replica->consensus_mutex);

            cluster_process(new_entry, &perform_voting, &on_no_results);

            status = RevPiDDS_InputDataReader_return_loan(input_DataReader, message_seq, message_infoSeq);
            checkStatus(status, "RevPiDDS_InputDataReader_return_loan");



        } else {
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