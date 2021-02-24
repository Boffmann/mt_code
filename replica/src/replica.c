#include "replica.h"

#include "dds_dcps.h"
#include "datamodel.h"
#include "DDSConsensusManager.h"

#include "backgroundTasks.h"

void *runElectionTimer(void* param) {

    replica_t* replica = (replica_t *)param;

    // uint32_t term_started = replica->current_term;
    DDS_Duration_t election_timeout = {1 * replica->ID + 1 , 0};
    DDS_sequence_RevPiDDS_AppendEntries msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_ReturnCode_t status;
    uint32_t received_Term = 0;

    while (true) {

        status = DDS_WaitSet_wait(appendEntries_WaitSet, appendEntries_GuardList, &election_timeout);

        pthread_mutex_lock(&replica->consensus_mutex);

        if (replica->role != FOLLOWER && replica->role != CANDIDATE) {
            printf("Election Timer triggered but I'm leader\n");
            pthread_mutex_unlock(&replica->consensus_mutex);
            break;
        }

        // if (term_started != replica->current_term) {
        //     printf("Election Timer triggered but term_started is outdated\n");
        //     pthread_mutex_unlock(&replica->consensus_mutex);
        //     break;
        // }

        if (status == DDS_RETCODE_TIMEOUT) {
            printf("No leader present in the system\n");
            pthread_mutex_unlock(&replica->consensus_mutex);
            status = DDS_GuardCondition_set_trigger_value(start_election_event, TRUE);
            checkStatus(status, "GGS_GuardCondition_set_trigger_value (start election TRUE)");
            break;
        } 

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
                        this_replica->voted_for = VOTED_NONE;
                    }
                }
            }
        }

        printf("Election Timer received with term %d\n", this_replica->current_term);
        pthread_mutex_unlock(&replica->consensus_mutex);
        RevPiDDS_AppendEntriesDataReader_return_loan(appendEntries_DataReader, &msgSeq, &infoSeq);

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
    checkStatus(status, "RevPiDDS_RequestVoteReplyDataWriter_write");

    DDS_free(heartbeat_message);
}

void become_leader(replica_t* replica) {

    pthread_mutex_lock(&replica->consensus_mutex);
    printf("Make leader\n");
    replica->role = LEADER;
    replica->voted_for = VOTED_NONE;

    if (setitimer(ITIMER_REAL, &replica->heartbeat_timer, NULL) == -1) {
        pthread_mutex_unlock(&replica->consensus_mutex);
        perror("Error calling setitimer()");
        return;
    }
    pthread_mutex_unlock(&replica->consensus_mutex);
}

void become_follower(replica_t* replica) {

    pthread_mutex_lock(&replica->consensus_mutex);
    printf("Make follower\n");
    replica->role = FOLLOWER;
    replica->voted_for = VOTED_NONE;

    if (pthread_create(&replica->election_timer_thread, NULL, runElectionTimer, replica) != 0) {
        printf("Error creating election timer thread\n");
        pthread_mutex_unlock(&replica->consensus_mutex);
        exit (-1);
    }

    pthread_mutex_unlock(&replica->consensus_mutex);
    printf("Became follower with term: %d\n", replica->current_term);

}

void initialize_replica(const uint8_t id) {
    replica_t *new_replica;

    DDSSetupConsensus();
    createElectionTimerDDSFeatures(id);
    createLeaderElectionDDSFeatures();

    new_replica = malloc(sizeof(replica_t));

    new_replica->ID = id;
    new_replica->current_term = 0;

    sigemptyset(&new_replica->heartbeat_action.sa_mask);
    new_replica->heartbeat_action.sa_sigaction = &send_heartbeat;
    sigaction (SIGALRM, &new_replica->heartbeat_action, NULL);

    new_replica->heartbeat_timer.it_value.tv_sec = 1;
    new_replica->heartbeat_timer.it_value.tv_usec = 0;

    new_replica->heartbeat_timer.it_interval.tv_sec = 1;
    new_replica->heartbeat_timer.it_interval.tv_usec = 0;

    pthread_mutex_init(&new_replica->consensus_mutex, NULL);

    become_follower(new_replica);

    this_replica = new_replica;

    pthread_create(&new_replica->leader_election_thread, NULL, run_leader_election_thread, NULL);
    pthread_create(&new_replica->request_vote_thread, NULL, receive_vote_requests, NULL);

}
