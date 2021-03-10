#ifndef __DDSCONSENSUSMANAGER_H__
#define __DDSCONSENSUSMANAGER_H__

#include "dds_dcps.h"
#include "DDSCreator/CheckStatus.h"
#include "DDSCreator/DDSEntitiesCreator.h"

DDS_char* electionTimer_QueryParameter;
DDS_char* electionTimer_QueryString;

DDS_Topic appendEntries_Topic, requestVote_Topic, requestVoteReply_Topic;
DDS_WaitSet appendEntries_WaitSet, collectVotes_WaitSet, leaderElection_WaitSet;
DDS_ReadCondition requestVote_ReadCondition, requestVoteReply_ReadCondition;
DDS_QueryCondition electionTimer_QueryCondition;
DDS_GuardCondition become_follower_event, become_leader_event, start_election_event;
DDS_Subscriber appendEntries_Subscriber, requestVote_Subscriber, requestVoteReply_Subscriber;
DDS_DataReader appendEntries_DataReader, requestVote_DataReader, requestVoteReply_DataReader;
DDS_Publisher appendEntries_Publisher, requestVote_Publisher, requestVoteReply_Publisher;
DDS_DataWriter appendEntries_DataWriter, requestVote_DataWriter, requestVoteReply_DataWriter;
DDS_ConditionSeq *appendEntries_GuardList, *collectVotes_GuardList, *leaderElection_GuardList;

void DDSSetupConsensus();
void DDSConsensusCleanup();

void createElectionTimerDDSFeatures(const uint8_t ID);
void createLeaderElectionDDSFeatures();

#endif