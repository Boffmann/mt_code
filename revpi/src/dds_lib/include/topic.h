#ifndef __RASPI_DDSLIB_TOPIC_H__
#define __RASPI_DDSLIB_TOPIC_H__

#include "dds_dcps.h"
#include "domain_participant.h"

/**
 * @brief This represents the topic's type
 * 
 * A TopicType indicates what the topic is used for and what kind of data is
 * expected to be in the topic.
 */
typedef enum {
    DECISIONS = 0,  ///< Topic used to store information about the actors in the system
    STATE = 1,      ///< Topic used to publish information about the system's current state
    TASKS = 2       ///< Topic used to publish information about tasks that should be processed by WORKERS
} TopicType;

/**
 * @brief Represents a topic.
 * 
 * Data can be published to this topic or it can be listened upon new data
 */
typedef struct {

    DDS_Topic dds_topic;    ///< The DDS topic that represents this topic in the DDS domain.
    const DDS_TopicQos* qos_handle;

} topic_t;

/**
 * @brief Join a new topic
 * 
 * This creates the topic with the specified role.
 * This method should be preferred over "topic_create_new"
 * 
 * @param domain_participant Handle to a participant in the same partition as topic
 * @param topic_qos The Quality of services that the topic should have
 * @param type The type of the newly cdreated topic
 * 
 * @return A new topic
 */
topic_t topic_join(domain_participant_t* domain_participant, const DDS_TopicQos* topic_qos, const TopicType type);

/**
 * @brief Create a new topic
 * 
 * @param domain_participant Handle to a participant in the same partition that this topic should be created in.
 * @param topicName Name of the new topic. Should be unique withing a partition
 * @param typeName Name of the type that this topic should store.
 * @param topic_qos The QoS settings that this topic should have
 * 
 * @return A new topic
 */
topic_t topic_create_new(domain_participant_t* domain_participant, const char* topicName, const char* typeName, const DDS_TopicQos* topic_qos);

/**
 * @brief Frees all resources kept by this topic
 * 
 * @param topic Handle to the topic to free
 * @param domain_participant Handle to a participant used to create this topic
 */
void topic_leave(topic_t* topic, domain_participant_t* domain_participant);



#endif