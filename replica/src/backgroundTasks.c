#include "backgroundTasks.h"

#include "dds_dcps.h"
#include "datamodel.h"
#include "DDSConsensusManager.h"

#include "replica.h"

void *collect_votes() {

    DDS_sequence_RevPiDDS_RequestVoteReply msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_ReturnCode_t status;
    uint32_t voteTerm = 0;
    uint8_t voted_candidate = 0;
    uint8_t sender_ID;
    bool voteGranted = false;
    DDS_Duration_t election_Timeout = {3, 0};
    uint32_t started_term;
    uint8_t votes_received = 1;

    bool is_collecting = true;

    started_term = this_replica->current_term;

    // TODO Waitset with read  event

    while (is_collecting) {

        status = DDS_WaitSet_wait(requestVoteReply_WaitSet, requestVoteReply_GuardList, &election_Timeout);

        if (status == DDS_RETCODE_TIMEOUT) {
            printf("Timeouted during waiting on vote collection\n");
            break;
        } 

        if (status != DDS_RETCODE_OK) {
            checkStatus(status, "DDS_WaitSet_wait (collectVotes Waitset)");
        }

        
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

            pthread_mutex_lock(&this_replica->consensus_mutex);
            for (DDS_unsigned_long i = 0; i < msgSeq._length; ++i) {
                if (infoSeq._buffer[i].valid_data) {
                    voteTerm = msgSeq._buffer[i].term;
                    voted_candidate = msgSeq._buffer[i].candidateID;
                    sender_ID = msgSeq._buffer[i].senderID;
                    voteGranted = msgSeq._buffer[i].voteGranted;
                    printf("Got some new requestVoteReply Data from %d - Term: %d Granted: %d\n", sender_ID, voteTerm, voteGranted);

                    if (this_replica->role != CANDIDATE) {
                        printf("Got a vote reply, but I'm not candidate anymore\n");
                        pthread_mutex_unlock(&this_replica->consensus_mutex);
                        RevPiDDS_RequestVoteReplyDataReader_return_loan(requestVoteReply_DataReader, &msgSeq, &infoSeq);
                        is_collecting = false;
                        break;
                    }

                    if (sender_ID == this_replica->ID) {
                        printf("Got a vote reply, but from myself\n");
                        pthread_mutex_unlock(&this_replica->consensus_mutex);
                        RevPiDDS_RequestVoteReplyDataReader_return_loan(requestVoteReply_DataReader, &msgSeq, &infoSeq);
                        continue;
                    }

                    if (voteTerm > started_term) {
                        printf("My term is out of date. Time to become Follower\n");
                        RevPiDDS_RequestVoteReplyDataReader_return_loan(requestVoteReply_DataReader, &msgSeq, &infoSeq);
                        status = DDS_GuardCondition_set_trigger_value(become_follower_event, TRUE);
                        checkStatus(status, "GGS_GuardCondition_set_trigger_value");
                        pthread_mutex_unlock(&this_replica->consensus_mutex);
                        is_collecting = false;
                        break;
                    }

                    if (voteGranted && voted_candidate == this_replica->ID) {
                        votes_received++;
                        printf("Vote for me got granted from %d. Got %d total votes\n", sender_ID, votes_received);
                        if (votes_received*2 > ACTIVE_REPLICAS) {
                            printf("Got enough votes. I am the new leader.\n");
                            RevPiDDS_RequestVoteReplyDataReader_return_loan(requestVoteReply_DataReader, &msgSeq, &infoSeq);
                            status = DDS_GuardCondition_set_trigger_value(become_leader_event, TRUE);
                            checkStatus(status, "GGS_GuardCondition_set_trigger_value");
                            pthread_mutex_unlock(&this_replica->consensus_mutex);
                            is_collecting = false;
                            break;
                        }
                    }
                }
            }
            pthread_mutex_unlock(&this_replica->consensus_mutex);
        }
        RevPiDDS_RequestVoteReplyDataReader_return_loan(requestVoteReply_DataReader, &msgSeq, &infoSeq);
    }

    printf("Exit the Vote collection thread\n");

    pthread_exit(NULL);
}

void *run_leader_election_thread() {

    RevPiDDS_RequestVote* requestVoteMessage;
    DDS_ReturnCode_t status;
    unsigned long condition_index = 0;
    uint8_t election_result = 0;
    pthread_t receiveVotes_thread;
    DDS_sequence_RevPiDDS_AppendEntries msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_Duration_t voting_Timeout = DDS_DURATION_INFINITE;
    DDS_Duration_t election_Timeout = {4 * this_replica->ID + 4 , 0};
    uint32_t received_Term = 0;


    while (true) {
        election_result = 0;
        condition_index = 0;
        printf("Wait for leader election to start\n");
        status = DDS_WaitSet_wait(election_WaitSet, election_GuardList, &voting_Timeout);
        printf("Start leader election\n");

        // TODO Ensure that the election timer has topped at this point

        status = DDS_GuardCondition_set_trigger_value(start_election_event, FALSE);
        checkStatus(status, "GGS_GuardCondition_set_trigger_value (start election FALSE)");

        pthread_mutex_lock(&this_replica->consensus_mutex);

        printf("Start new leader election. Voted for: %d\n", this_replica->voted_for);
        if (this_replica->voted_for == VOTED_NONE || this_replica->voted_for == this_replica->ID) {
            printf("Make Candidate\n");
            this_replica->role = CANDIDATE;
            this_replica->current_term++;
            this_replica->voted_for = this_replica->ID;
        }

        requestVoteMessage = RevPiDDS_RequestVote__alloc();
        requestVoteMessage->term = this_replica->current_term;
        requestVoteMessage->candidateID = this_replica->voted_for;
        requestVoteMessage->senderID = this_replica->ID;

        printf("About to write onto RequestVote Topic\n");
        status = RevPiDDS_RequestVoteDataWriter_write(requestVote_DataWriter, requestVoteMessage, DDS_HANDLE_NIL);
        checkStatus(status, "RevPiDDS_RequestVoteDataWriter_write");

        pthread_mutex_unlock(&this_replica->consensus_mutex);


        // A candidate continues in this state until one of three things happen
        // 1: It wins the election
        // 2: Another module establishes itself as leader
        // 3: It timeouts without a winner
        // Wait until at least one of the conditions in the waitset trigger

        if (pthread_create(&receiveVotes_thread, NULL, collect_votes, this_replica) != 0) {
            // TODO
        }

        printf("Wait until new leader elected or timeout\n");
        status = DDS_WaitSet_wait(collectVotes_WaitSet, collectVotes_GuardList, &election_Timeout);

        if (status == DDS_RETCODE_OK) {
            // Check what exactly happened
            do {
                if ( collectVotes_GuardList->_buffer[condition_index] == become_leader_event) {
                    election_result |= WON_ELECTION;
                    status = DDS_GuardCondition_set_trigger_value(become_leader_event, FALSE);
                    checkStatus(status, "GGS_GuardCondition_set_trigger_value (become_leader FALSE)");
                } else if (collectVotes_GuardList->_buffer[condition_index] == become_follower_event) {
                    printf("Received a become follower event\n");
                    election_result |= LOST_ELECTION;
                    status = DDS_GuardCondition_set_trigger_value(become_follower_event, FALSE);
                    checkStatus(status, "GGS_GuardCondition_set_trigger_value (become_follower FALSE)");
                } else if (collectVotes_GuardList->_buffer[condition_index] == electionTimer_QueryCondition) {
                    printf("Received an election timer\n");
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
                        pthread_mutex_lock(&this_replica->consensus_mutex);
                        for (DDS_unsigned_long i = 0; i < msgSeq._length; ++i) {
                            if (infoSeq._buffer[i].valid_data) {
                                received_Term = msgSeq._buffer[i].term;
                                if (received_Term > this_replica->current_term) {
                                    this_replica->current_term = received_Term;
                                    this_replica->voted_for = VOTED_NONE;
                                    election_result |= LOST_ELECTION;
                                }
                            }
                        }
                        pthread_mutex_unlock(&this_replica->consensus_mutex);
                        RevPiDDS_AppendEntriesDataReader_return_loan(appendEntries_DataReader, &msgSeq, &infoSeq);
                    }
                }
            } while ( ++condition_index < collectVotes_GuardList->_length);

        } else if (status == DDS_RETCODE_TIMEOUT) {
            election_result |= TIMEOUT;
        } else {
            // Some error happened.
            checkStatus(status, "DDS_WaitSet_wait collectVotes Waitset");
        }

        if (election_result & LOST_ELECTION) {
            printf("I am becoming a follower\n");
            become_follower(this_replica);
            printf("I am a follower\n");
        } else if (election_result & WON_ELECTION) {
            printf("I am becoming a leader\n");
            become_leader(this_replica);
            printf("I am a leader\n");
        } else {
            printf("Election needs to be restarted\n");
            // pthread_join(&receiveVotes_thread, NULL);
            this_replica->voted_for = VOTED_NONE;
            status = DDS_GuardCondition_set_trigger_value(start_election_event, TRUE);
            checkStatus(status, "GGS_GuardCondition_set_trigger_value (Restart election)");
        }

        DDS_free(requestVoteMessage);

    }
    pthread_exit(NULL);

}

void *receive_vote_requests() {
    DDS_ReturnCode_t status;
    DDS_Duration_t timeout = DDS_DURATION_INFINITE;
    DDS_sequence_RevPiDDS_RequestVote msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    RevPiDDS_RequestVoteReply* requestVoteReplyMessage;
    bool voteGranted = false;
    uint32_t voteTerm = 0;
    uint8_t voteCandidate = 0;
    uint8_t sender_ID = 0;
    bool gotValidAndNewData = false;

    while (true) {
        voteGranted = false;
        gotValidAndNewData = false;

        status = DDS_WaitSet_wait(requestVote_WaitSet, requestVote_GuardList, &timeout);

        // TODO Check the read condition

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
                    voteCandidate = msgSeq._buffer[i].candidateID;
                    sender_ID = msgSeq._buffer[i].senderID;
                    printf("Got this: voteTerm: %d candidate: %d\n", voteTerm, voteCandidate);
                    if (voteCandidate != this_replica->ID) {
                        printf("Got some new requestVote Data from %d - voteTerm: %d candidate: %d\n", sender_ID, voteTerm, voteCandidate);

                        pthread_mutex_lock(&this_replica->consensus_mutex);

                        if (voteTerm >= this_replica->current_term && (this_replica->voted_for == VOTED_NONE || this_replica->voted_for == voteCandidate)) {
                            this_replica->voted_for = voteCandidate;
                            voteGranted = true;

                        }

                        requestVoteReplyMessage = RevPiDDS_RequestVoteReply__alloc();
                        requestVoteReplyMessage->term = this_replica->current_term;
                        requestVoteReplyMessage->candidateID = this_replica->voted_for;
                        requestVoteReplyMessage->voteGranted = voteGranted;
                        requestVoteReplyMessage->senderID = this_replica->ID;

                        if (this_replica->role == LEADER) {
                            pthread_mutex_unlock(&this_replica->consensus_mutex);
                            become_follower(this_replica);
                        } else {
                            pthread_mutex_unlock(&this_replica->consensus_mutex);
                        }

                        if (voteGranted) {
                            printf("About to grant vote for replica %d with term %d\n", sender_ID, this_replica->current_term);
                        } else {
                            printf("About to decline vote for replica %d with term %d because I already voted for %d\n", sender_ID, this_replica->current_term, this_replica->voted_for);
                        }
                        status = RevPiDDS_RequestVoteReplyDataWriter_write(requestVoteReply_DataWriter, requestVoteReplyMessage, DDS_HANDLE_NIL);
                        checkStatus(status, "RevPiDDS_RequestVoteReplyDataWriter_write");
                        //  TODO Error in RevPiDDS_RequestVoteReplyDataWriter_write: DDS_RETCODE_OUT_OF_RESOURCES in term 7

                        DDS_free(requestVoteReplyMessage);
                    }
                }
            }
        }

        RevPiDDS_RequestVoteDataReader_return_loan(requestVote_DataReader, &msgSeq, &infoSeq);

        if (gotValidAndNewData) {

        } 
    }

}