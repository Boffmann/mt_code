#ifndef __RASPI_DDSLIB_TOPIC_H__
#define __RASPI_DDSLIB_TOPIC_H__

#include "dds_dcps.h"
#include "domain_participant.h"

typedef enum {
    ACTORS = 0,
    TASKS = 1
} TopicType;

typedef struct {

    DDS_Topic dds_topic;

} topic_t;

topic_t topic_create_new(domain_participant_t* domain_participant, const char* topicName, const char* typeName, DDS_TopicQos* topic_qos);
void topic_leave(topic_t* topic, domain_participant_t* domain_participant);



#endif