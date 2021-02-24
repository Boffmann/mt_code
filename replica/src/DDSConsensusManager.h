#ifndef __DDSCONSENSUSMANAGER_H__
#define __DDSCONSENSUSMANAGER_H__

#include "dds_dcps.h"
#include "CheckStatus.h"
#include "DDSEntitiesManager.h"

DDS_Topic appendEntries_Topic, requestVote_Topic, requestVoteReply_Topic;
DDS_WaitSet appendEntries_WaitSet, requestVote_WaitSet, requestVoteReply_WaitSet, collectVotes_WaitSet, election_WaitSet;
DDS_ReadCondition electionTimer_ReadCondition, requestVote_ReadCondition, requestVoteReply_ReadCondition;
DDS_GuardCondition become_follower_event, become_leader_event, start_election_event;
DDS_Subscriber appendEntries_Subscriber, requestVote_Subscriber, requestVoteReply_Subscriber;
DDS_DataReader appendEntries_DataReader, requestVote_DataReader, requestVoteReply_DataReader;
DDS_Publisher appendEntries_Publisher, requestVote_Publisher, requestVoteReply_Publisher;
DDS_DataWriter appendEntries_DataWriter, requestVote_DataWriter, requestVoteReply_DataWriter;
DDS_ConditionSeq *appendEntries_GuardList, *requestVote_GuardList, *requestVoteReply_GuardList, *collectVotes_GuardList, *election_GuardList;
// struct DDS_DataReaderListener *appendEntries_Listener, *requestVote_Listener, *requestVoteReply_Listener, *replicaResult_Listener;

void DDSSetupConsensus();
void DDSConsensusCleanup();

void createAppendEntriesTopic();
void createRequestVoteTopic();
void createRequestVoteReplyTopic();

void createElectionTimerDDSFeatures();
void createLeaderElectionDDSFeatures();
void createReceiveVotesDDSFeatures();

#endif