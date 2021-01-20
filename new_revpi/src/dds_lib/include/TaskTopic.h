#ifndef __TASKTOPIC_H__
#define __TASKTOPIC_H__

#include "dds_dcps.h"
#include "datamodel.h"

#include <stdbool.h>

struct TaskMessage {
    DDS_sequence_RevPiDDS_Task* message_seq;
    DDS_SampleInfoSeq* message_infoSeq;
};

typedef struct {
    long taskType;
} task_data_t;

typedef enum {

    NONE = 0,
    SHUTDOWN = 1,
    SPEED_MONITORING = 2

} TaskType;

typedef void (*on_task_data_available_t)(const task_data_t* data);

extern on_task_data_available_t task_data_available_callback;

DDS_Topic taskTopic_create(DDS_DomainParticipant domainParticipant, const char* topicName);

DDS_PublisherQos* taskTopic_getPublisherQos(DDS_DomainParticipant domainParticipant);
DDS_DataWriterQos* taskTopic_getDataWriterQos(DDS_Publisher publisher, DDS_Topic topic);

DDS_SubscriberQos* taskTopic_getSubscriberQos(DDS_DomainParticipant domainParticipant);
DDS_DataReaderQos* taskTopic_getDataReaderQos(DDS_Subscriber subscriber, DDS_Topic topic);

void taskTopic_registerListener(struct DDS_DataReaderListener* listener, DDS_DataReader dataReader, on_task_data_available_t callback);

void taskTopic_newMessage(struct TaskMessage* message);
void taskTopic_freeMessage(struct TaskMessage* message);

void taskTopic_write(DDS_DataWriter dataWriter, const task_data_t* data);
bool taskTopic_read(DDS_DataReader dataReader, struct TaskMessage* message, task_data_t* result_data);

#endif