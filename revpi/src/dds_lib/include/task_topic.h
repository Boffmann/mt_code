#ifndef __RASPI_DDSLIB_TASKTOPIC_H__
#define __RASPI_DDSLIB_TASKTOPIC_H__

#include "dds_dcps.h"
#include "topic.h"
#include "domain_participant.h"
#include "listener.h"

typedef struct {
    long task_id;
    char* message;
} task_t;

typedef struct {
    DDS_DataReader* task_data_reader;
} task_listener_data_t;

typedef void (*on_task_data_available_t)(const task_t* task_data);

extern on_task_data_available_t on_task_data_available_callback;

topic_t tasks_topic_create(const domain_participant_t* domain_participant);
void task_topic_listen(listener_t* listener, on_task_data_available_t callback);

#endif