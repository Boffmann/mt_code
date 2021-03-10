#include "replica.h"

#include <time.h>
#include <stdlib.h>

#include "datamodel.h"
#include "DDSConsensusManager.h"
#include "DIO/DIO.h"
#include "leaderElection.h"

void *runElectionTimer(void* param) {

    replica_t* replica = (replica_t *)param;

    DDS_sequence_RevPiDDS_AppendEntries msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_ReturnCode_t status;
    uint32_t received_Term = 0;

    while (true) {

        status = DDS_WaitSet_wait(appendEntries_WaitSet, appendEntries_GuardList, &this_replica->election_timeout);

        pthread_mutex_lock(&replica->consensus_mutex);


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
                        if (received_Term > this_replica->current_term) {
                            this_replica->current_term = received_Term;
                            become_follower(this_replica);
                        }
                    }
                }
            }

            printf("Election Timer received with term %d\n", this_replica->current_term);
            pthread_mutex_unlock(&replica->consensus_mutex);
            RevPiDDS_AppendEntriesDataReader_return_loan(appendEntries_DataReader, &msgSeq, &infoSeq);
        } else {
            if (replica->role != FOLLOWER && replica->role != CANDIDATE) {
                printf("Election Timer timeouted but I'm leader\n");
                pthread_mutex_unlock(&replica->consensus_mutex);
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
    RevPiDDS_AppendEntries* heartbeat_message;

    heartbeat_message = RevPiDDS_AppendEntries__alloc();
    heartbeat_message->term = this_replica->current_term;
    heartbeat_message->senderID = this_replica->ID;

    printf("About to send heartbeat with term %d\n", heartbeat_message->term);
    status = RevPiDDS_AppendEntriesDataWriter_write(appendEntries_DataWriter, heartbeat_message, DDS_HANDLE_NIL);
    checkStatus(status, "RevPiDDS_AppendEntriesDataWriter_write heartbeat message");

    DDS_free(heartbeat_message);
}

void become_leader(replica_t* replica) {

    if (replica->role == LEADER) {
        return;
    }

    printf("Make leader\n");
    digital_write("O_1", 0);
    digital_write("O_2", 1);
    replica->role = LEADER;
    replica->voted_for = VOTED_NONE;

    if (setitimer(ITIMER_REAL, &replica->heartbeat_timer, NULL) == -1) {
        perror("Error calling setitimer()");
        return;
    }
}

void become_follower(replica_t* replica) {

    printf("Make follower\n");
    digital_write("O_1", 1);
    digital_write("O_2", 0);
    replica->role = FOLLOWER;
    replica->voted_for = VOTED_NONE;
    DDS_unsigned_long random_timeout = (rand() % 300000000) + 700000000;
    replica->election_timeout.sec = 0;
    replica->election_timeout.nanosec = random_timeout;

    if (setitimer(ITIMER_REAL, NULL, NULL) == -1) {
        perror("Error calling setitimer()");
        return;
    }

    printf("Became follower with term: %d\n", replica->current_term);

}

void initialize_replica(const uint8_t id) {
    replica_t *new_replica;

    DDSSetupConsensus();
    createElectionTimerDDSFeatures(id);
    createLeaderElectionDDSFeatures();

    srand(time(NULL));

    new_replica = malloc(sizeof(replica_t));

    new_replica->ID = id;
    new_replica->current_term = 0;

    sigemptyset(&new_replica->heartbeat_action.sa_mask);
    new_replica->heartbeat_action.sa_sigaction = &send_heartbeat;
    sigaction (SIGALRM, &new_replica->heartbeat_action, NULL);

    new_replica->heartbeat_timer.it_value.tv_sec = 0;
    new_replica->heartbeat_timer.it_value.tv_usec = 50000;

    new_replica->heartbeat_timer.it_interval.tv_sec = 0;
    new_replica->heartbeat_timer.it_interval.tv_usec = 50000;

    pthread_mutex_init(&new_replica->consensus_mutex, NULL);

    become_follower(new_replica);

    this_replica = new_replica;

    if (pthread_create(&new_replica->election_timer_thread, NULL, runElectionTimer, new_replica) != 0) {
        printf("Error creating election timer thread\n");
        exit (-1);
    }

    pthread_create(&new_replica->request_vote_thread, NULL, receive_vote_requests, NULL);

}

void teardown_replica() {
    DDSConsensusCleanup();

    pthread_cancel(this_replica->request_vote_thread);

    free(this_replica);
}