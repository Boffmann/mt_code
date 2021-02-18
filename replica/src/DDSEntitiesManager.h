#ifndef __DDSENTITIESMANAGER_H__
#define __DDSENTITIESMANAGER_H__

#include "dds_dcps.h"
#include "CheckStatus.h"
#include "DDSEntitiesCreator.h"
#include "datamodel.h"

extern DDS_DomainParticipant domainParticipant;

DDS_Topic input_Topic, appendEntries_Topic, requestVote_Topic, requestVoteReply_Topic;
DDS_WaitSet input_WaitSet, appendEntries_WaitSet, leaderElection_WaitSet;
DDS_ReadCondition input_ReadCondition, appendEntries_ReadCondition;
DDS_GuardCondition won_election;
DDS_Subscriber input_Subscriber, appendEntries_Subscriber, requestVote_Subscriber, requestVoteReply_Subscriber;
DDS_DataReader input_DataReader, appendEntries_DataReader, requestVote_DataReader, requestVoteReply_DataReader;
DDS_Publisher appendEntries_Publisher, requestVote_Publisher, requestVoteReply_Publisher;
DDS_DataWriter appendEntries_DataWriter, requestVote_DataWriter, requestVoteReply_DataWriter;
DDS_ConditionSeq *input_GuardList, *appendEntries_GuardList, *election_GuardList;
struct DDS_DataReaderListener *appendEntries_Listener, *requestVote_Listener, *requestVoteReply_Listener;

extern const char* partitionName;

void DDSSetup();
void DDSCleanup();

void createInputTopic();
void createAppendEntriesTopic();
void createRequestVoteTopic();
void createRequestVoteReplyTopic();


#endif