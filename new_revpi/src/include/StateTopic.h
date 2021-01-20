#ifndef __STATETOPIC_H__
#define __STATETOPIC_H__

#include "dds_dcps.h"
#include "datamodel.h"

// extern char* g_StateTypeName;
// extern DDS_TypeSupport g_StateTypeSupport;

DDS_Topic stateTopic_create(DDS_DomainParticipant domainParticipant, const char* topicName);

DDS_PublisherQos* stateTopic_getPublisherQos(DDS_DomainParticipant domainParticipant);
DDS_DataWriterQos* stateTopic_getDataWriterQos(DDS_Publisher publisher, DDS_Topic topic);

DDS_SubscriberQos* stateTopic_getSubscriberQos(DDS_DomainParticipant domainParticipant);
DDS_DataReaderQos* stateTopic_getDataReaderQos(DDS_Subscriber publisher, DDS_Topic topic);

void stateTopic_write(DDS_DataWriter dataWriter);

#endif