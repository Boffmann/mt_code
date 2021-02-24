#ifndef __DDSENTITIESMANAGER_H__
#define __DDSENTITIESMANAGER_H__

#include "dds_dcps.h"
#include "CheckStatus.h"
#include "DDSEntitiesCreator.h"
#include "datamodel.h"

extern DDS_DomainParticipant domainParticipant;

DDS_Topic input_Topic, replicaResult_Topic;
DDS_WaitSet input_WaitSet;
DDS_ReadCondition input_ReadCondition;
DDS_Subscriber input_Subscriber, replicaResult_Subscriber;
DDS_DataReader input_DataReader, replicaResult_DataReader;
DDS_Publisher replicaResult_Publisher;
DDS_DataWriter replicaResult_DataWriter;
DDS_ConditionSeq *input_GuardList;
// struct DDS_DataReaderListener *appendEntries_Listener, *requestVote_Listener, *requestVoteReply_Listener, *replicaResult_Listener;

extern DDS_DomainParticipant domainParticipant;
extern const char* partitionName;

void DDSSetup();
void DDSCleanup();

void createInputTopic();
void createReplicaResultTopic();


#endif