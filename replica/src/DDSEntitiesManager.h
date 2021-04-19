#ifndef __DDSENTITIESMANAGER_H__
#define __DDSENTITIESMANAGER_H__

#include "dds_dcps.h"
#include "DDSCreator/CheckStatus.h"
#include "DDSCreator/DDSEntitiesCreator.h"
#include "datamodel.h"


DDS_Topic input_Topic;
DDS_WaitSet input_WaitSet;
DDS_ReadCondition input_ReadCondition;
DDS_Subscriber input_Subscriber;
DDS_DataReader input_DataReader;
DDS_Publisher input_Publisher;
DDS_DataWriter input_DataWriter;
DDS_ConditionSeq *input_GuardList;
// struct DDS_DataReaderListener *appendEntries_Listener, *requestVote_Listener, *requestVoteReply_Listener, *replicaResult_Listener;

extern DDS_DomainParticipant domainParticipant;
extern const char* partitionName;

void DDSSetup();
void DDSCleanup();

void createInputTopic();
void createReplicaResultTopic();


#endif