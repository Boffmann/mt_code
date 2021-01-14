#ifndef __DDSLIB_API_H__
#define __DDSLIB_API_H__

/*
 ******************************
 * PUBLIC API FOR THE DDS_LIB *
 ******************************
*/

#include "domain_participant.h"
#include "actor.h"
#include "topic.h"
#include "publisher.h"
#include "listener.h"

/**
 * Sets up the entire DDS domain.
 * This includes the DDS domain, as well as involved DDS domain participants.
 */
domain_participant_t setup_dds_domain(char* partition_name);

topic_t join_topic(domain_participant_t* domain_participant, const DDS_TopicQos* topic_qos, const TopicType type);

publisher_t add_publisher(const domain_participant_t* domain_participant, const topic_t* topic);

listener_t add_listener(const domain_participant_t* domain_participant, const topic_t* topic);

DDS_TopicQos* get_default_topic_qos(const domain_participant_t* domain_participant);

#endif