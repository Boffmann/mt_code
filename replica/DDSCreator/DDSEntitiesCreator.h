#ifndef __DDSENTITIESCREATOR_H__
#define __DDSENTITIESCREATOR_H__

#include "dds_dcps.h"

extern DDS_DomainId_t g_domainID;
extern DDS_DomainParticipantFactory g_domainParticipantFactory;
extern DDS_DomainParticipant domainParticipant;

extern const char* g_partitionName;

extern DDS_ReturnCode_t g_status;

DDS_DomainParticipant createParticipant(const char* partitionName);

void deleteParticipant(DDS_DomainParticipant domainParticipant);

// register Type

DDS_Topic createTopic(DDS_DomainParticipant domainParticipant, const char* topicName, const char* typeName, const DDS_TopicQos* topicQos);
void deleteTopic(DDS_DomainParticipant domainParticipant, DDS_Topic topic);

DDS_Publisher createPublisher(DDS_DomainParticipant domainParticipant, const DDS_PublisherQos* publisherQos);
void deletePublisher(DDS_DomainParticipant domainParticipant, DDS_Publisher publisher);

DDS_DataWriter createDataWriter(DDS_Publisher publisher, DDS_Topic topic, DDS_DataWriterQos* dataWriterQos);
void deleteDataWriter(DDS_Publisher publisher, DDS_DataWriter dataWriter);

DDS_Subscriber createSubscriber(DDS_DomainParticipant domainParticipant, const DDS_SubscriberQos* subscriberQos);
void deleteSubscriber(DDS_DomainParticipant domainParticipant, DDS_Subscriber subscriber);

DDS_DataReader createDataReader(DDS_Subscriber subscriber, DDS_Topic topic, DDS_DataReaderQos* dataReaderQos);
void deleteDataReader(DDS_Subscriber subscriber, DDS_DataReader dataReader);

struct DDS_DataReaderListener* createDataReaderListener();
void deleteDataReaderListener(struct DDS_DataReaderListener* listener);


#endif