#ifndef __STATETOPIC_H__
#define __STATETOPIC_H__

#include "dds_dcps.h"
#include "datamodel.h"

#include <stdbool.h>

struct StateMessage {
    DDS_sequence_RevPiDDS_State* message_seq;
    DDS_SampleInfoSeq* message_infoSeq;
};

typedef struct {
    long timestamp;
    double speed;
} state_data_t;

typedef void (*on_state_data_available_t)(const state_data_t* state_data);

extern on_state_data_available_t state_data_available_callback;

DDS_Topic stateTopic_create(DDS_DomainParticipant domainParticipant, const char* topicName);

DDS_PublisherQos* stateTopic_getPublisherQos(DDS_DomainParticipant domainParticipant);
DDS_DataWriterQos* stateTopic_getDataWriterQos(DDS_Publisher publisher, DDS_Topic topic);

DDS_SubscriberQos* stateTopic_getSubscriberQos(DDS_DomainParticipant domainParticipant);
DDS_DataReaderQos* stateTopic_getDataReaderQos(DDS_Subscriber subscriber, DDS_Topic topic);

void stateTopic_registerListener(struct DDS_DataReaderListener* listener, DDS_DataReader dataReader, on_state_data_available_t callback);

void stateTopic_newMessage(struct StateMessage* message);
void stateTopic_freeMessage(struct StateMessage* message);

void stateTopic_write(DDS_DataWriter dataWriter, const state_data_t* data);
bool stateTopic_read(DDS_DataReader dataReader, struct StateMessage* message, state_data_t* result_data);

#endif