#include "leaderElection.h"

#include "dds_dcps.h"
#include "datamodel.h"
#include "DDSConsensusManager.h"

#include "replica.h"

uint8_t votes_received = 0;

void run_leader_election() {

    RevPiDDS_RequestVote* requestVoteMessage;
    DDS_ReturnCode_t status;
    unsigned long condition_index = 0;
    DDS_sequence_RevPiDDS_AppendEntries msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_Duration_t election_Timeout = {4 * this_replica->ID + 4 , 0};
    uint32_t received_Term = 0;
    bool election_finished = false;

    while (!election_finished) {
        condition_index = 0;
        printf("Start leader election\n");

        printf("Start new leader election. Voted for: %d\n", this_replica->voted_for);
        printf("Make Candidate\n");
        this_replica->role = CANDIDATE;
        this_replica->current_term++;
        this_replica->election_term = this_replica->current_term;
        this_replica->voted_for = this_replica->ID;
        votes_received = 1;

        requestVoteMessage = RevPiDDS_RequestVote__alloc();
        requestVoteMessage->term = this_replica->current_term;
        requestVoteMessage->senderID = this_replica->voted_for;

        printf("About to write onto RequestVote Topic\n");
        status = RevPiDDS_RequestVoteDataWriter_write(requestVote_DataWriter, requestVoteMessage, DDS_HANDLE_NIL);
        checkStatus(status, "RevPiDDS_RequestVoteDataWriter_write");

        pthread_mutex_unlock(&this_replica->consensus_mutex);


        // A candidate continues in this state until one of three things happen
        // 1: It wins the election
        // 2: Another module establishes itself as leader
        // 3: It timeouts without a winner
        // Wait until at least one of the conditions in the waitset trigger
        printf("Wait until new leader elected or timeout\n");
        status = DDS_WaitSet_wait(leaderElection_WaitSet, leaderElection_GuardList, &election_Timeout);

        if (status == DDS_RETCODE_OK) {
            printf("Ok. Got a valid event in leader election waitset\n");
            // Check what exactly happened
            do {
                if (leaderElection_GuardList->_buffer[condition_index] == electionTimer_QueryCondition) {
                    printf("Received an election timer\n");
                    status = RevPiDDS_AppendEntriesDataReader_take(
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
                        pthread_mutex_lock(&this_replica->consensus_mutex);
                        for (DDS_unsigned_long i = 0; i < msgSeq._length; ++i) {
                            if (infoSeq._buffer[i].valid_data) {
                                received_Term = msgSeq._buffer[i].term;
                                if (received_Term > this_replica->current_term) {
                                    printf("I am becoming a follower\n");
                                    become_follower(received_Term);
                                    printf("I am a follower\n");
                                }
                                if (received_Term == this_replica->current_term) {
                                    if (this_replica->role != FOLLOWER) {
                                        become_follower(received_Term);
                                    }
                                    election_finished = true;
                                }
                            }
                        }
                        pthread_mutex_unlock(&this_replica->consensus_mutex);
                        RevPiDDS_AppendEntriesDataReader_return_loan(appendEntries_DataReader, &msgSeq, &infoSeq);
                    }
                } else {
                    printf("Nothing from the above\n");
                }
            } while ( ++condition_index < collectVotes_GuardList->_length);

        } else if (status == DDS_RETCODE_TIMEOUT) {
            printf("Leader election timeouted\n");
            election_finished = true;
        } else {
            // Some error happened.
            checkStatus(status, "DDS_WaitSet_wait collectVotes Waitset");
        }

        DDS_free(requestVoteMessage);

    }

}

void handle_vote_message() {
    DDS_ReturnCode_t status;
    DDS_sequence_RevPiDDS_RequestVote msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    uint32_t voteTerm = 0;
    uint8_t voteCandidate = 0;
    RevPiDDS_RequestVoteReply* requestVoteReplyMessage;
    bool voteGranted = false;

    status = RevPiDDS_RequestVoteDataReader_take(
        requestVote_DataReader,
        &msgSeq,
        &infoSeq,
        DDS_LENGTH_UNLIMITED,
        DDS_NOT_READ_SAMPLE_STATE,
        DDS_ANY_VIEW_STATE,
        DDS_ANY_INSTANCE_STATE
    );
    checkStatus(status, "RevPiDDS_RequestVoteDataReader_take");

    if (msgSeq._length > 0) {
        for (DDS_unsigned_long i = 0; i < msgSeq._length; ++i) {
            if (infoSeq._buffer[i].valid_data) {
                voteTerm = msgSeq._buffer[i].term;
                voteCandidate = msgSeq._buffer[i].senderID;
                printf("Got this: voteTerm: %d candidate: %d\n", voteTerm, voteCandidate);

                printf("Got some new requestVote Data from %d - voteTerm: %d candidate: %d my term: %d \n", voteCandidate, voteTerm, voteCandidate, this_replica->current_term);

                if (voteTerm > this_replica->current_term) {
                    become_follower(voteTerm);
                }

                if (voteTerm == this_replica->current_term &&
                    (this_replica->voted_for == VOTED_NONE || this_replica->voted_for == voteCandidate)) {
                    this_replica->voted_for = voteCandidate;
                    this_replica->current_term = voteTerm;
                    voteGranted = true;
                }

                requestVoteReplyMessage = RevPiDDS_RequestVoteReply__alloc();
                requestVoteReplyMessage->term = this_replica->current_term;
                requestVoteReplyMessage->candidateID = this_replica->voted_for;
                requestVoteReplyMessage->voteGranted = voteGranted;
                requestVoteReplyMessage->senderID = this_replica->ID;

                if (voteGranted) {
                    printf("About to grant vote for replica %d with my current term %d\n", voteCandidate, this_replica->current_term);
                } else {
                    printf("About to decline vote for replica %d with term %d because I already voted for %d\n", voteCandidate, this_replica->current_term, this_replica->voted_for);
                }
                status = RevPiDDS_RequestVoteReplyDataWriter_write(requestVoteReply_DataWriter, requestVoteReplyMessage, DDS_HANDLE_NIL);
                checkStatus(status, "RevPiDDS_RequestVoteReplyDataWriter_write");
                //  TODO Error in RevPiDDS_RequestVoteReplyDataWriter_write: DDS_RETCODE_OUT_OF_RESOURCES in term 7

                DDS_free(requestVoteReplyMessage);
            }
        }
    }

    RevPiDDS_RequestVoteDataReader_return_loan(requestVote_DataReader, &msgSeq, &infoSeq);

}

void handle_vote_reply_message() {

    DDS_sequence_RevPiDDS_RequestVoteReply msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_ReturnCode_t status;
    uint32_t voteTerm = 0;
    uint8_t voted_candidate = 0;
    uint8_t sender_ID;
    bool voteGranted = false;

    // TODO Waitset with read  event

    printf("Got a vote reply\n");

    status = RevPiDDS_RequestVoteReplyDataReader_take(
        requestVoteReply_DataReader,
        &msgSeq,
        &infoSeq,
        DDS_LENGTH_UNLIMITED,
        DDS_NOT_READ_SAMPLE_STATE,
        DDS_ANY_VIEW_STATE,
        DDS_ANY_INSTANCE_STATE
    );

    checkStatus(status, "RevPiDDS_RequestVoteReplyDataReader_take");
    if (msgSeq._length > 0) {

        for (DDS_unsigned_long i = 0; i < msgSeq._length; ++i) {
            if (infoSeq._buffer[i].valid_data) {
                voteTerm = msgSeq._buffer[i].term;
                voted_candidate = msgSeq._buffer[i].candidateID;
                sender_ID = msgSeq._buffer[i].senderID;
                voteGranted = msgSeq._buffer[i].voteGranted;
                printf("Got some new requestVoteReply Data from %d - Term: %d Granted: %d voted for: %d\n", sender_ID, voteTerm, voteGranted, voted_candidate);
                
                if (this_replica->role != CANDIDATE) {
                    printf("Got a vote reply, but I'm not candidate anymore\n");
                    pthread_mutex_unlock(&this_replica->consensus_mutex);
                    RevPiDDS_RequestVoteReplyDataReader_return_loan(requestVoteReply_DataReader, &msgSeq, &infoSeq);
                    break;
                }

                if (voteTerm > this_replica->election_term) {
                    printf("My term is out of date. Time to become Follower\n");
                    RevPiDDS_RequestVoteReplyDataReader_return_loan(requestVoteReply_DataReader, &msgSeq, &infoSeq);
                    become_follower(voteTerm);
                    break;
                }

                if (voteGranted && voteTerm == this_replica->election_term && voted_candidate == this_replica->ID) {
                    votes_received++;
                    printf("Vote for me got granted from %d. Got %d total votes\n", sender_ID, votes_received);
                    if (votes_received*2 > ACTIVE_REPLICAS) {
                        printf("Got enough votes. I am the new leader.\n");
                        RevPiDDS_RequestVoteReplyDataReader_return_loan(requestVoteReply_DataReader, &msgSeq, &infoSeq);
                        become_leader(this_replica);
                        break;
                    }
                }
            }
        }
    }
    RevPiDDS_RequestVoteReplyDataReader_return_loan(requestVoteReply_DataReader, &msgSeq, &infoSeq);

}

void *receive_vote_requests() {
    DDS_ReturnCode_t status;
    DDS_Duration_t timeout = DDS_DURATION_INFINITE;
    unsigned long guardList_index;

    while (true) {

        status = DDS_WaitSet_wait(collectVotes_WaitSet, collectVotes_GuardList, &timeout);

        if (status == DDS_RETCODE_OK) {
            guardList_index = 0;
            do {
                if (collectVotes_GuardList->_buffer[guardList_index] == requestVoteReply_QueryCondition) {
                    pthread_mutex_lock(&this_replica->consensus_mutex);
                    handle_vote_reply_message();
                    pthread_mutex_unlock(&this_replica->consensus_mutex);
                } else if (collectVotes_GuardList->_buffer[guardList_index] == requestVote_QueryCondition) {
                    pthread_mutex_lock(&this_replica->consensus_mutex);
                    handle_vote_message();
                    pthread_mutex_unlock(&this_replica->consensus_mutex);
                }
            } while ( ++guardList_index < collectVotes_GuardList->_length);

        }
    }
}