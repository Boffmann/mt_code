#ifndef __RASPI_DDSLIB_TOPIC_H__
#define __RASPI_DDSLIB_TOPIC_H__

#include "dds_dcps.h"
#include "domain_participant.h"

typedef enum {
    ACTORS = 0
} TopicType;

typedef struct {

    DDS_Topic dds_topic;

} topic_t;

// REGION Topic behaviour functions
topic_t topic_create_new(domain_participant_t* domain_participant, const char* topicName, const char* typeName, DDS_TopicQos* topic_qos);
void topic_leave(topic_t* topic, domain_participant_t* domain_participant);

// REGION Publisher behaviour functions
// struct topic_publisher_t topic_publisher_create_new(const topic_t* topic);
// void topic_publisher_delete(const struct topic_publisher_t* publisher);


#endif