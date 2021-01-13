#ifndef _DDSENTITIESMANAGER_H_
#define _DDSENTITIESMANAGER_H_

#include "dds_dcps.h"
#include "datamodel.h"

// DDS entities and other variables
extern DDS_DomainId_t g_domainId;
extern DDS_DomainParticipantFactory g_domainParticipantFactory;
extern DDS_DomainParticipant g_domainParticipant;

extern const char* g_partitionName;

// Examples's Topics
extern char* g_MessageTypeName;
extern DDS_TypeSupport g_MessageTypeSupport;
extern DDS_Topic g_MessageTopic;

// Error handling
extern DDS_ReturnCode_t g_status;

// Declare handy helper functions
void createParticipant(const char * partitiontName);

void deleteParticipant();

void registerMessageType(DDS_TypeSupport typeSupport);

DDS_Topic createTopic(const char *topicName, const char *typeName);

void deleteTopic(DDS_Topic topic);

DDS_Publisher createPublisher();

void deletePublisher(DDS_Publisher publisher);

DDS_DataWriter createDataWriter(DDS_Publisher publisher, DDS_Topic topic);

void setOwnershipStrength(DDS_DataWriter dataWriter, DDS_long strength);

void deleteDataWriter(DDS_Publisher publisher, DDS_DataWriter dataWriter);

DDS_Subscriber createSubscriber();

void deleteSubscriber(DDS_Subscriber subscriber);

DDS_DataReader createDataReader(DDS_Subscriber subscriber, DDS_Topic topic);

void deleteDataReader(DDS_Subscriber subscriber, DDS_DataReader dataReader);

#endif
