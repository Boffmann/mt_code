#ifndef __RASPI_DDSLIB_STATETOPIC_H__
#define __RASPI_DDSLIB_STATETOPIC_H__

#include "dds_dcps.h"
#include "topic.h"
#include "domain_participant.h"
#include "listener.h"
#include "publisher.h"

// Type forward declaration
// struct state_listener_data_t;

typedef struct {
    long timestamp;
    double speed;
} state_t;

typedef struct {
    RevPiDDS_State *message;
    DDS_InstanceHandle_t message_handle;
} dds_state_instance_t;

typedef void (*on_state_data_available_t)(const state_t* state_data);

extern on_state_data_available_t on_state_data_available_callback;

topic_t state_topic_create(const domain_participant_t* domain_participant, const DDS_TopicQos* topic_qos);

dds_state_instance_t state_topic_create_new_instance(const publisher_t* publisher);
void state_topic_dispose_instance(const publisher_t* publisher, dds_state_instance_t* state_instance);

void state_topic_publish(const publisher_t* publisher, dds_state_instance_t* state_instance);

bool state_topic_read(const subscriber_t* subscriber, state_t* state_data);

void state_topic_listen(listener_t* listener, on_state_data_available_t callback);

#endif