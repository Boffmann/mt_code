#include "replica.h"

#include <time.h>
#include <stdlib.h>

#include "DDSConsensusManager.h"
#include "DIO/DIO.h"
#include "datamodel.h"
#include "leaderElection.h"

void *runElectionTimer() {

    DDS_sequence_RevPiDDS_AppendEntries msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    RevPiDDS_AppendEntriesReply* appendEntriesReply_message;
    DDS_ReturnCode_t status;
    uint32_t received_Term = 0;
    int received_prevLogIndex = 0;
    uint32_t received_prevLogTerm = 0;
    uint32_t received_leaderCommit = 0;
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
                        received_prevLogIndex = msgSeq._buffer[i].prevLogIndex;
                        received_prevLogTerm = msgSeq._buffer[i].prevLogTerm;
                        received_leaderCommit = msgSeq._buffer[i].leaderCommit;
                        entries_Seq = msgSeq._buffer[i].entries;
                        if (received_Term > this_replica->current_term) {
                            become_follower(received_Term);
                        }

                        bool success = false;
                        if (received_Term == this_replica->current_term) {
                            if (this_replica->role != FOLLOWER) {
                                become_follower(received_Term);
                            }

                            if (received_prevLogIndex == -1 ||
                                ((received_prevLogIndex == -1 || (size_t)received_prevLogIndex < this_replica->log_size) && received_prevLogTerm == this_replica->log[received_prevLogIndex].term)) {
                                success = true;
                                uint32_t logInsertIndex = received_prevLogIndex + 1;
                                uint32_t newEntryIndex = 0;
                                while (true) {
                                    if (logInsertIndex >= this_replica->log_size || newEntryIndex >= this_replica->log_size) {
                                        break;
                                    }
                                    logInsertIndex++;
                                    newEntryIndex++;
                                }

                                if (newEntryIndex < this_replica->log_size) {
                                    printf("Inserting entries from index %d\n", logInsertIndex);
                                    for (DDS_unsigned_long entry_index = 0; entry_index < entries_Seq._length; ++entry_index) {
                                        log_entry_t entry;
                                        entry.id = entries_Seq._buffer[entry_index].id;
                                        entry.term = received_Term;
                                        insert_log_entry_at(entry, logInsertIndex + entry_index);
                                    }
                                }

                                if (received_leaderCommit > this_replica->commitIndex) {
                                    this_replica->commitIndex = fmin(received_leaderCommit, this_replica->log_size-1);
                                    // TODO new Commit Ready
                                }
                            }
                        }

                        appendEntriesReply_message = RevPiDDS_AppendEntriesReply__alloc();
                        appendEntriesReply_message->senderID = this_replica->ID;
                        appendEntriesReply_message->term = this_replica->current_term;
                        appendEntriesReply_message->success = success;
                        pthread_mutex_unlock(&this_replica->consensus_mutex);

                        status = RevPiDDS_AppendEntriesReplyDataWriter_write(appendEntriesReply_DataWriter, appendEntriesReply_message, DDS_HANDLE_NIL);
                        checkStatus(status, "RevPiDDS_AppendEntriesReplyDataWriter write AppendEntriesReply");
                        DDS_free(appendEntriesReply_message);
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

    pthread_mutex_lock(&this_replica->consensus_mutex);
    int prevLogIndex = this_replica->nextIndex - 1;
    int prevLogTerm = -1;

    if (prevLogIndex >= 0) {
        prevLogTerm = this_replica->log[prevLogIndex].term;
    }

    appendEntries_message = RevPiDDS_AppendEntries__alloc();
    appendEntries_message->term = this_replica->current_term;
    appendEntries_message->senderID = this_replica->ID;
    appendEntries_message->prevLogIndex = prevLogIndex;
    appendEntries_message->prevLogTerm = prevLogTerm;
    appendEntries_message->senderID = this_replica->commitIndex;
    // TODO Add actual entries
    pthread_mutex_unlock(&this_replica->consensus_mutex);

    printf("About to send heartbeat with term %d\n", appendEntries_message->term);
    status = RevPiDDS_AppendEntriesDataWriter_write(appendEntries_DataWriter, appendEntries_message, DDS_HANDLE_NIL);
    checkStatus(status, "RevPiDDS_AppendEntriesDataWriter_write heartbeat message");
     // TODO Write appendEntriesReply handler

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
    
    this_replica->nextIndex = this_replica->log_size + 1;

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

    // TODO Adjust heartbeat timer.
    // There should be a timeout to when the final result should be made by the followers
    // When this exeeds, trigger safety mechanisms
    // Heartbeat timer should be a little above this timeout
    this_replica->heartbeat_timer.it_value.tv_sec = 0;
    this_replica->heartbeat_timer.it_value.tv_usec = 50000;

    this_replica->heartbeat_timer.it_interval.tv_sec = 0;
    this_replica->heartbeat_timer.it_interval.tv_usec = 50000;

    this_replica->election_timeout.sec = 0;
    this_replica->election_timeout.nanosec = random_timeout;

    this_replica->log_size = 0;
    this_replica->log = malloc(this_replica->log_size);
    this_replica->commitIndex = 0;
    this_replica->lastApplied = 0;
    this_replica->matchIndex = 0;
    
    pthread_mutex_init(&this_replica->consensus_mutex, NULL);

    become_follower(0);

    if (pthread_create(&this_replica->election_timer_thread, NULL, runElectionTimer, NULL) != 0) {
        printf("Error creating election timer thread\n");
        exit (-1);
    }

    pthread_create(&this_replica->request_vote_thread, NULL, receive_vote_requests, NULL);

}

void append_to_log(const log_entry_t entry) {
    this_replica->log_size++;
    this_replica->log = realloc(this_replica->log, this_replica->log_size * sizeof(log_entry_t));
    this_replica->log[this_replica->log_size] = entry;
}

void insert_log_entry_at(log_entry_t entry, size_t index) {
    if (index + 1 > this_replica->log_size) {
        this_replica->log_size += this_replica->log_size - index + 1;
        this_replica->log = realloc(this_replica->log, this_replica->log_size * sizeof(log_entry_t));
    }
    this_replica->log[index] = entry;
}

void teardown_replica() {
    DDSConsensusCleanup();

    pthread_cancel(this_replica->request_vote_thread);

    free(this_replica->log);
    free(this_replica);
}