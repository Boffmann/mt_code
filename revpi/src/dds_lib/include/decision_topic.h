#ifndef __RASPI_DDSLIB_DECISIONTOPIC_H__
#define __RASPI_DDSLIB_DECISIONTOPIC_H__

#include <stdbool.h>

#include "dds_dcps.h"
#include "topic.h"
#include "domain_participant.h"
#include "publisher.h"
#include "listener.h"

// Type forward declaration
// struct decision_listener_data_t;

typedef struct {
    long decision_id;
} decision_t;

typedef void (*on_decision_data_available_t)(const decision_t* decision);

extern on_decision_data_available_t on_decision_data_available_callback;

topic_t decision_topic_create(const domain_participant_t* domain_participant, const DDS_TopicQos* topic_qos);

void decision_topic_publish(const publisher_t* publisher, const decision_t* decision_data);

bool decision_topic_read(const subscriber_t* subscriber, decision_t* decision_data);

void decision_topic_listen(listener_t* listener, on_decision_data_available_t callback);
#endif