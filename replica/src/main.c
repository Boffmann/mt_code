#include <stdbool.h>

#include <signal.h>
#include <sys/time.h>

#include "dds_dcps.h"
#include "CheckStatus.h"
#include "DDSEntitiesManager.h"
#include "role.h"
#include "datamodel.h"

#define VOTED_NONE 255


enum {
    WON_ELECTION        = (1 << 0),
    LEADER_ACTIVE       = (1 << 1),
    TIMEOUT             = (1 << 2)
};


RaftRole role;
struct sigaction heartbeat_action;
struct itimerval heartbeat_timer;
uint32_t current_term;
uint8_t voted_for;
uint8_t replica_ID;
uint8_t votes_received;
uint8_t active_replicas = 2;
bool leader_active;

void send_heartbeat(int signum) {
    (void) signum;

    DDS_ReturnCode_t status;
    RevPiDDS_AppendEntries* heartbeat_message;


    // TODO
    heartbeat_message = RevPiDDS_AppendEntries__alloc();
    heartbeat_message->term = current_term;

    printf("About to send heartbeat\n");
    status = RevPiDDS_AppendEntriesDataWriter_write(appendEntries_DataWriter, heartbeat_message, DDS_HANDLE_NIL);
    checkStatus(status, "RevPiDDS_RequestVoteReplyDataWriter_write");

    DDS_free(heartbeat_message);
}

void heartbeat_missed_callback(void* listener_data, DDS_DataReader reader, const DDS_RequestedDeadlineMissedStatus *status) {
    (void) listener_data;
    (void) reader;
    (void) status;

    printf("Leader appears to be dead\n");
    leader_active = false;

}

void on_entry_available_callback(void* listener_data, DDS_DataReader reader) {
    (void) listener_data;
    (void) reader;
    // DDS_sequence_RevPiDDS_AppendEntries msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    // DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    // DDS_ReturnCode_t status;

    printf("Leader alive and kicking\n");
    leader_active = true;

    // TODO Check that this message is actually a confirmation conditions 
    // status = DDS_GuardCondition_set_trigger_value(voting_confirmed, TRUE);
    // checkStatus(status, "DDS_GuardCondition_set_trigger_value");
}

void on_received_vote_reply_callback(void* listener_data, DDS_DataReader reader) {
    (void) listener_data;
    (void) reader;
    DDS_sequence_RevPiDDS_RequestVoteReply msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_ReturnCode_t status;
    uint32_t voteTerm = 0;
    uint8_t voted_candidate = 0;
    bool voteGranted = false;

    printf("Got a vote reply\n");

    // TODO read incoming vote
    status = RevPiDDS_RequestVoteReplyDataReader_take(
        requestVoteReply_DataReader,
        &msgSeq,
        &infoSeq,
        1,
        DDS_ANY_SAMPLE_STATE,
        DDS_ANY_VIEW_STATE,
        DDS_ANY_INSTANCE_STATE
    );
    checkStatus(status, "RevPiDDS_RequestVoteReplyDataReader_take");

    if (msgSeq._length > 0 && infoSeq._buffer[0].valid_data) {
        voteTerm = msgSeq._buffer[0].term;
        voted_candidate = msgSeq._buffer[0].candidateID;
        voteGranted = msgSeq._buffer[0].voteGranted;
        printf("Got some new requestVoteReply Data - Term: %d Granted: %d\n", voteTerm, voteGranted);

        if (voteGranted && voted_candidate == replica_ID) {
            printf("Vote for me got granted\n");
            votes_received++;
            if (votes_received*2 > active_replicas) {
                printf("Got enough votes. I am the new leader. Set trigger value\n");
                status = DDS_GuardCondition_set_trigger_value(won_election, TRUE);
                checkStatus(status, "GGS_GuardCondition_set_trigger_value");
            }
        }
        if (voteTerm > current_term) {
            current_term = voteTerm;
        }
    }

    RevPiDDS_RequestVoteReplyDataReader_return_loan(requestVoteReply_DataReader, &msgSeq, &infoSeq);

}

void on_received_vote_request_callback(void* listener_data, DDS_DataReader reader) {
    (void) listener_data;
    (void) reader;
    DDS_sequence_RevPiDDS_RequestVote msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    RevPiDDS_RequestVoteReply* requestVoteReplyMessage;
    DDS_ReturnCode_t status;
    bool voteGranted = false;
    uint32_t voteTerm = 0;
    uint8_t voteCandidate = 0;
    bool gotValidAndNewData = false;

    printf("Vote Incoming\n");

    // TODO read incoming vote
    status = RevPiDDS_RequestVoteDataReader_take(
        requestVote_DataReader,
        &msgSeq,
        &infoSeq,
        1,
        DDS_ANY_SAMPLE_STATE,
        DDS_ANY_VIEW_STATE,
        DDS_ANY_INSTANCE_STATE
    );
    checkStatus(status, "RevPiDDS_RequestVoteDataReader_take");

    if (msgSeq._length > 0 && infoSeq._buffer[0].valid_data) {
        voteTerm = msgSeq._buffer[0].term;
        voteCandidate = msgSeq._buffer[0].candidateID;
        printf("Got this: voteTerm: %d candidate: %d\n", voteTerm, voteCandidate);
        if (voteCandidate != replica_ID && role == CANDIDATE) {
            gotValidAndNewData = true;
        }
    }

    RevPiDDS_RequestVoteDataReader_return_loan(requestVote_DataReader, &msgSeq, &infoSeq);

    if (gotValidAndNewData) {
        printf("Got some new requestVote Data - voteTerm: %d candidate: %d\n", voteTerm, voteCandidate);

        if (voteTerm >= current_term) {
            voteGranted = true;
        }
        if (voted_for == VOTED_NONE || voted_for == voteCandidate) {
            voted_for = voteCandidate;
            voteGranted = true;
        }

        requestVoteReplyMessage = RevPiDDS_RequestVoteReply__alloc();
        requestVoteReplyMessage->term = current_term;
        requestVoteReplyMessage->candidateID = voted_for;
        requestVoteReplyMessage->voteGranted = voteGranted;

        printf("About to write onto RequestVoteReply Topic\n");
        status = RevPiDDS_RequestVoteReplyDataWriter_write(requestVoteReply_DataWriter, requestVoteReplyMessage, DDS_HANDLE_NIL);
        checkStatus(status, "RevPiDDS_RequestVoteReplyDataWriter_write");

        DDS_free(requestVoteReplyMessage);
    }
}

void become_follower(const uint32_t term) {
    DDS_StatusMask mask;
    DDS_StatusMask requestVoteListener_Mask;
    DDS_StatusMask requestVoteReplyListener_Mask;
    DDS_ReturnCode_t status;

    role = FOLLOWER;
    current_term = term;

    printf("Became follower with term: %d\n", term);

    // Wait for incoming vote requests
    requestVote_Listener->listener_data = NULL;
    requestVote_Listener->on_data_available = on_received_vote_request_callback;

    // TODO Validate mask
    requestVoteListener_Mask = DDS_DATA_AVAILABLE_STATUS;
    status = DDS_DataReader_set_listener(requestVote_DataReader, requestVote_Listener, requestVoteListener_Mask);
    checkStatus(status, "DDS_DataReader_set_listener (RequestVote)");
    
    // Start election timer
    appendEntries_Listener->listener_data = NULL;
    appendEntries_Listener->on_data_available = on_entry_available_callback;
    appendEntries_Listener->on_requested_deadline_missed = heartbeat_missed_callback;

    // TODO Validate mask
    mask = DDS_REQUESTED_DEADLINE_MISSED_STATUS | DDS_DATA_AVAILABLE_STATUS;
    status = DDS_DataReader_set_listener(appendEntries_DataReader, appendEntries_Listener, mask);
    checkStatus(status, "DDS_DataReader_set_listener (AppendEntries)");

    requestVoteReply_Listener->listener_data = NULL;
    requestVoteReply_Listener->on_data_available = NULL;

    requestVoteReplyListener_Mask = DDS_DATA_AVAILABLE_STATUS;
    status = DDS_DataReader_set_listener(requestVoteReply_DataReader, requestVoteReply_Listener, requestVoteReplyListener_Mask);
    checkStatus(status, "DDS_DataReader_set_listener (requestVoteReply)");
}

void become_leader() {
    DDS_StatusMask mask;
    DDS_StatusMask requestVoteListener_Mask;
    DDS_StatusMask requestVoteReplyListener_Mask;
    DDS_ReturnCode_t status;

    role = LEADER;

    appendEntries_Listener->on_data_available = NULL;
    appendEntries_Listener->on_requested_deadline_missed = NULL;

    mask = DDS_REQUESTED_DEADLINE_MISSED_STATUS | DDS_DATA_AVAILABLE_STATUS;
    status = DDS_DataReader_set_listener(appendEntries_DataReader, appendEntries_Listener, mask);
    checkStatus(status, "DDS_DataReader_set_listener (AppendEntries)");

    requestVote_Listener->listener_data = NULL;
    requestVote_Listener->on_data_available = NULL;

    // TODO Validate mask
    requestVoteListener_Mask = DDS_DATA_AVAILABLE_STATUS;
    status = DDS_DataReader_set_listener(requestVote_DataReader, requestVote_Listener, requestVoteListener_Mask);
    checkStatus(status, "DDS_DataReader_set_listener (RequestVote)");

    requestVoteReply_Listener->listener_data = NULL;
    requestVoteReply_Listener->on_data_available = NULL;

    requestVoteReplyListener_Mask = DDS_DATA_AVAILABLE_STATUS;
    status = DDS_DataReader_set_listener(requestVoteReply_DataReader, requestVoteReply_Listener, requestVoteReplyListener_Mask);
    checkStatus(status, "DDS_DataReader_set_listener (requestVoteReply)");

    if (setitimer(ITIMER_REAL, &heartbeat_timer, NULL) == -1) {
        perror("Error calling setitimer()");
    }
}

void elect_new_leader() {

    // uint8_t votes_received = 0;
    RevPiDDS_RequestVote* requestVoteMessage;
    DDS_ReturnCode_t status;
    DDS_Duration_t election_timeout = {10, 0};
    DDS_StatusMask requestVoteReplyListener_Mask;
    unsigned long condition_index = 0;
    uint8_t election_result = 0;

    printf("Start new leader election\n");
    current_term++;
    role = CANDIDATE;
    // TODO reset election timer
    voted_for = replica_ID;

    votes_received = 1;


    requestVoteMessage = RevPiDDS_RequestVote__alloc();
    requestVoteMessage->term = current_term;
    requestVoteMessage->candidateID = replica_ID;

    printf("About to write onto RequestVote Topic\n");
    status = RevPiDDS_RequestVoteDataWriter_write(requestVote_DataWriter, requestVoteMessage, DDS_HANDLE_NIL);
    checkStatus(status, "RevPiDDS_RequestVoteDataWriter_write");

    requestVoteReply_Listener->listener_data = NULL;
    requestVoteReply_Listener->on_data_available = on_received_vote_reply_callback;

    requestVoteReplyListener_Mask = DDS_DATA_AVAILABLE_STATUS;
    status = DDS_DataReader_set_listener(requestVoteReply_DataReader, requestVoteReply_Listener, requestVoteReplyListener_Mask);
    checkStatus(status, "DDS_DataReader_set_listener (requestVoteReply)");

    // A candidate continues in this state until one of three things happen
    // 1: It wins the election
    status = DDS_WaitSet_attach_condition(leaderElection_WaitSet, won_election);
    checkStatus(status, "DDS_WaitSet_attach_condition (guardCondition)");
    // 2: Another module establishes itself as leader
    status = DDS_WaitSet_attach_condition(leaderElection_WaitSet, appendEntries_ReadCondition);
    checkStatus(status, "DDS_WaitSet_attach_condition (readCondition)");
    // 3: It timeouts without a winner

    // Wait until at least one of the conditions in the waitset trigger
    printf("Wait until new leader elected or timeout\n");
    status = DDS_WaitSet_wait(leaderElection_WaitSet, election_GuardList, &election_timeout);

    if (status == DDS_RETCODE_OK) {
        if (role != CANDIDATE) {
            printf("Got a new leader, but I'm not a candidate\n");
            return;
        }

        // Check what exactly happened
        do {
            if ( election_GuardList->_buffer[condition_index] == won_election) {
                election_result |= WON_ELECTION;
                status = DDS_GuardCondition_set_trigger_value(won_election, FALSE);
                checkStatus(status, "GGS_GuardCondition_set_trigger_value");
            }
            else if ( election_GuardList->_buffer[condition_index] == appendEntries_ReadCondition) {
                election_result |= LEADER_ACTIVE;
            }
        } while ( ++condition_index < election_GuardList->_length);

    } else if (status == DDS_RETCODE_TIMEOUT) {
        election_result |= TIMEOUT;
    } else {
        // Some error happened.
        checkStatus(status, "DDS_WaitSet_wait");
    }

    if (election_result & LEADER_ACTIVE) {
        become_follower(current_term);
        printf("I am a follower\n");
    } else if (election_result & WON_ELECTION) {
        become_leader();
        printf("I am a leader\n");
    } else {
        // TODO Restart election
        printf("Election needs to be restarted\n");
    }

    DDS_free(requestVoteMessage);

}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: %s [replicaID]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    replica_ID = (uint8_t)atoi(argv[1]);

    DDS_ReturnCode_t status;
    uint8_t input_index = 0;
    unsigned long i = 0;
    DDS_sequence_RevPiDDS_Input* message_seq = DDS_sequence_RevPiDDS_Input__alloc();
    DDS_SampleInfoSeq* message_infoSeq = DDS_SampleInfoSeq__alloc();
    DDS_Duration_t input_Timeout = DDS_DURATION_INFINITE;
    // TODO randomize
    const DDS_Duration_t election_timeout = {1*replica_ID, 0};

    voted_for = VOTED_NONE;

    memset(&heartbeat_action, 0, sizeof(heartbeat_action));
    heartbeat_action.sa_handler = &send_heartbeat;
    sigaction (SIGALRM, &heartbeat_action, NULL);

    heartbeat_timer.it_value.tv_sec = 1;
    heartbeat_timer.it_value.tv_usec = 0;

    heartbeat_timer.it_interval.tv_sec = 1;
    heartbeat_timer.it_interval.tv_usec = 0;

    DDSSetup();


    become_follower(0);

    // At first, validate whether there is a leader in the cluster
    status = DDS_WaitSet_attach_condition(appendEntries_WaitSet, appendEntries_ReadCondition);
    checkStatus(status, "DDS_WaitSet_attach_condition (appendEntries_ReadCondition)");
    status = DDS_WaitSet_wait(appendEntries_WaitSet, appendEntries_GuardList, &election_timeout);

    if (status == DDS_RETCODE_TIMEOUT) {
        printf("No leader present in the system\n");
        elect_new_leader();
    } 

    status = DDS_WaitSet_detach_condition(appendEntries_WaitSet, appendEntries_ReadCondition);
    checkStatus(status, "DDS_WaitSet_detach_condition");
    status = RevPiDDS_AppendEntriesDataReader_delete_readcondition(appendEntries_DataReader, appendEntries_ReadCondition);
    checkStatus(status, "RevPiDDS_AppendEntriesDataReader_delete_readcondition");

    status = DDS_WaitSet_attach_condition(input_WaitSet, input_ReadCondition);
    checkStatus(status, "DDS_WaitSet_attach_condition (input_ReadCondition)");

    status = DDS_WaitSet_wait(input_WaitSet, input_GuardList, &input_Timeout);


    if (status == DDS_RETCODE_OK) {
        input_index = 0;
        do {
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
            status = RevPiDDS_InputDataReader_return_loan(input_DataReader, message_seq, message_infoSeq);
            checkStatus(status, "RevPiDDS_InputDataReader_return_loan");

        } while ( ++input_index < input_GuardList->_length);

    } else {
        checkStatus(status, "DDS_WaitSet_wait");
    }

    status = DDS_WaitSet_detach_condition(input_WaitSet, input_ReadCondition);
    checkStatus(status, "DDS_WaitSet_detach_condition");

    status = RevPiDDS_InputDataReader_delete_readcondition(input_DataReader, input_ReadCondition);
    checkStatus(status, "RevPiDDS_InputDataReader_delete_readcondition");

    DDSCleanup();
    DDS_free(message_seq);
    DDS_free(message_infoSeq);
}