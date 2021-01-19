#ifndef __DDSENTITIESMANAGER_H__
#define __DDSENTITIESMANAGER_H__

#include "dds_dcps.h"
#include "datamodel.h"

extern DDS_DomainId_t g_domainID;
extern DDS_DomainParticipantFactory g_domainParticipantFactory;
extern DDS_DomainParticipant g_domainParticipant;

extern const char* g_partitionName;


extern DDS_ReturnCode_t g_status;

void createParticipant(const char* partitionName);

void deleteParticipant();

// register Type

DDS_Topic createTopic(const char* topicName, const char* typeName, const DDS_TopicQos* topicQos);

void deleteTopic(DDS_Topic topic);

DDS_Publisher createPublisher(const DDS_PublisherQos* publisherQos);

void deletePublisher(DDS_Publisher publisher);

DDS_DataWriter createDataWriter(DDS_Publisher publisher, DDS_Topic topic, DDS_DataWriterQos* dataWriterQos);

void deleteDataWriter(DDS_Publisher publisher, DDS_DataWriter dataWriter);


#endif