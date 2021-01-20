#ifndef __DECISIONTOPIC_H__
#define __DECISIONTOPIC_H__

#include "dds_dcps.h"
#include "datamodel.h"

#include <stdbool.h>

struct DecisionMessage {
    DDS_sequence_RevPiDDS_Decision* message_seq;
    DDS_SampleInfoSeq* message_infoSeq;
};

typedef struct {
    long senderID;
    long decision;
} decision_data_t;

typedef void (*on_decision_data_available_t)(const decision_data_t* data);

extern on_decision_data_available_t decision_data_available_callback;

DDS_Topic decisionTopic_create(DDS_DomainParticipant domainParticipant, const char* topicName);

DDS_PublisherQos* decisionTopic_getPublisherQos(DDS_DomainParticipant domainParticipant);
DDS_DataWriterQos* decisionTopic_getDataWriterQos(DDS_Publisher publisher, DDS_Topic topic);

DDS_SubscriberQos* decisionTopic_getSubscriberQos(DDS_DomainParticipant domainParticipant);
DDS_DataReaderQos* decisionTopic_getDataReaderQos(DDS_Subscriber subscriber, DDS_Topic topic);

void decisionTopic_registerListener(struct DDS_DataReaderListener* listener, DDS_DataReader dataReader, on_decision_data_available_t callback);

void decisionTopic_newMessage(struct DecisionMessage* message);
void decisionTopic_freeMessage(struct DecisionMessage* message);

void decisionTopic_write(DDS_DataWriter dataWriter, const decision_data_t* data);
bool decisionTopic_read(DDS_DataReader dataReader, struct DecisionMessage* message, decision_data_t* result_data);

#endif