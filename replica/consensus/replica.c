#include "replica.h"

#include <time.h>
#include <stdlib.h>

#include "datamodel.h"
#include "DDSConsensusManager.h"
#include "DIO/DIO.h"
#include "leaderElection.h"

void *runElectionTimer() {

    DDS_sequence_RevPiDDS_AppendEntries msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_ReturnCode_t status;
    uint32_t received_Term = 0;
    DDS_sequence_RevPiDDS_Entry entries_Seq;

    while (true) {

        status = DDS_WaitSet_wait(appendEntries_WaitSet, appendEntries_GuardList, &this_replica->election_timeout);

        pthread_mutex_lock(&this_replica->consensus_mutex);

        if (status != DDS_RETCODE_TIMEOUT) {
            status = RevPiDDS_AppendEntriesDataReader_read(
                appendEntries_DataReader,
                &msgSeq,
                &infoSeq,
                DDS_LENGTH_UNLIMITED,
                DDS_NOT_READ_SAMPLE_STATE,
                DDS_ANY_VIEW_STATE,
                DDS_ALIVE_INSTANCE_STATE
            );
            checkStatus(status, "RevPiDDS_AppendEntriesDataReader_read (election Timer)");
            if (msgSeq._length > 0) {
                for (DDS_unsigned_long i = 0; i < msgSeq._length; ++i) {
                    if (infoSeq._buffer[i].valid_data) {
                        received_Term = msgSeq._buffer[i].term;
                        entries_Seq = msgSeq._buffer[i].entries;
                        if (received_Term > this_replica->current_term) {
                            become_follower(received_Term);
                        }

                        if (entries_Seq._length > 0) {
                            printf("Got some new entries\n");
                            for (DDS_unsigned_long entry_index = 0; entry_index < entries_Seq._length; ++entry_index) {
                                uint32_t id = entries_Seq._buffer[entry_index].id;
                                printf("Got the following entry: ID: %d\n", id);
                            }
                        }
                    }
                }
            }

            printf("Election Timer received with term %d\n", this_replica->current_term);
            pthread_mutex_unlock(&this_replica->consensus_mutex);
            RevPiDDS_AppendEntriesDataReader_return_loan(appendEntries_DataReader, &msgSeq, &infoSeq);
        } else {
            if (this_replica->role != FOLLOWER && this_replica->role != CANDIDATE) {
                printf("Election Timer timeouted but I'm leader\n");
                pthread_mutex_unlock(&this_replica->consensus_mutex);
                continue;
            }
            printf("No leader present in the system\n");
            run_leader_election();
        }

    }

    pthread_exit(NULL);
}

void send_heartbeat(int signum, siginfo_t* info, void* args) {
    (void) signum;
    (void) info;
    (void) args;

    DDS_ReturnCode_t status;
    RevPiDDS_AppendEntries* appendEntries_message;

    appendEntries_message = RevPiDDS_AppendEntries__alloc();
    pthread_mutex_lock(&this_replica->consensus_mutex);
    appendEntries_message->term = this_replica->current_term;
    appendEntries_message->senderID = this_replica->ID;
    uint8_t payload_size = this_replica->log_tail;
    appendEntries_message->entries._length = payload_size;
    appendEntries_message->entries._maximum = payload_size;
    appendEntries_message->entries._buffer = DDS_sequence_RevPiDDS_Entry_allocbuf(payload_size);
    this_replica->log_tail = 0;
    for (uint8_t i = 0; i < payload_size; i++) {
        printf("Appending an entry to appendEntry Message\n");
        appendEntries_message->entries._buffer[i].id = this_replica->log[i].id;
    }
    pthread_mutex_unlock(&this_replica->consensus_mutex);

    printf("About to send heartbeat with term %d\n", appendEntries_message->term);
    status = RevPiDDS_AppendEntriesDataWriter_write(appendEntries_DataWriter, appendEntries_message, DDS_HANDLE_NIL);
    checkStatus(status, "RevPiDDS_AppendEntriesDataWriter_write appendEntries_message");

    DDS_free(appendEntries_message);
}

void become_leader() {

    if (this_replica->role == LEADER) {
        return;
    }

    printf("Make leader\n");
    digital_write("O_1", 0);
    digital_write("O_2", 1);
    this_replica->role = LEADER;
    this_replica->voted_for = VOTED_NONE;

    if (setitimer(ITIMER_REAL, &this_replica->heartbeat_timer, NULL) == -1) {
        perror("Error calling setitimer()");
        return;
    }
}

void become_follower(const uint32_t term) {

    printf("Make follower\n");
    digital_write("O_1", 1);
    digital_write("O_2", 0);
    this_replica->current_term = term;
    this_replica->role = FOLLOWER;
    this_replica->voted_for = VOTED_NONE;

    if (setitimer(ITIMER_REAL, NULL, NULL) == -1) {
        perror("Error calling setitimer()");
        return;
    }

    printf("Became follower with term: %d\n", this_replica->current_term);

}

void initialize_replica(const uint8_t id) {

    DDSSetupConsensus();
    createLeaderElectionDDSFeatures(id);

    srand(time(NULL));
    DDS_unsigned_long random_timeout = (rand() % 300000000) + 700000000;

    this_replica = malloc(sizeof(replica_t));

    this_replica->ID = id;

    sigemptyset(&this_replica->heartbeat_action.sa_mask);
    this_replica->heartbeat_action.sa_sigaction = &send_heartbeat;
    sigaction (SIGALRM, &this_replica->heartbeat_action, NULL);

    this_replica->heartbeat_timer.it_value.tv_sec = 0;
    this_replica->heartbeat_timer.it_value.tv_usec = 50000;

    this_replica->heartbeat_timer.it_interval.tv_sec = 0;
    this_replica->heartbeat_timer.it_interval.tv_usec = 50000;

    this_replica->election_timeout.sec = 0;
    this_replica->election_timeout.nanosec = random_timeout;

    this_replica->log_tail = 0;
    // this_replica->log = malloc(LOG_PUFFER * sizeof(log_entry_t));

    pthread_mutex_init(&this_replica->consensus_mutex, NULL);

    become_follower(0);

    if (pthread_create(&this_replica->election_timer_thread, NULL, runElectionTimer, NULL) != 0) {
        printf("Error creating election timer thread\n");
        exit (-1);
    }

    pthread_create(&this_replica->request_vote_thread, NULL, receive_vote_requests, NULL);

}

bool append_to_log(const log_entry_t entry) {
    if (this_replica->log_tail >= LOG_PUFFER) {
        return false;
    }

    this_replica->log[this_replica->log_tail] = entry;
    this_replica->log_tail++;

    return true;
}

void teardown_replica() {
    DDSConsensusCleanup();

    pthread_cancel(this_replica->request_vote_thread);

    free(this_replica);
}