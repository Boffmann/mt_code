#ifndef __DDSENTITIESMANAGER_H__
#define __DDSENTITIESMANAGER_H__

#include "dds_dcps.h"
#include "DDSCreator/CheckStatus.h"
#include "DDSCreator/DDSEntitiesCreator.h"
#include "datamodel.h"


/**
 * DDS Entities used for reading and processing inputs
 */
DDS_Topic input_Topic;
DDS_WaitSet input_WaitSet;
DDS_ReadCondition input_ReadCondition;
DDS_Subscriber input_Subscriber;
DDS_DataReader input_DataReader;
DDS_Publisher input_Publisher;
DDS_DataWriter input_DataWriter;
DDS_ConditionSeq *input_GuardList;

// The DDS Domain participant for the replica
extern DDS_DomainParticipant domainParticipant;
// The partition name where the participant is part of
extern const char* partitionName;

/**
 * Setup the input processing DDS Entities
 */
void DDSSetup();

/**
 * Cleanup all input processing DDS entities and release memory
 */
void DDSCleanup();


#endif