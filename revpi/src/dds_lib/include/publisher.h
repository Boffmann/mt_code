#ifndef __RASPI_DDSLIB_PUBLISHER_H__
#define __RASPI_DDSLIB_PUBLISHER_H__

#include <stdbool.h>

#include "dds_dcps.h"
#include "domain_participant.h"
#include "topic.h"

/**
 * @brief Represents a DDS publisher that can publish new messages to a topic
 * 
 * This structure combines a DDS publisher and DataWriter used to publish new messages onto a topic.
 * It is possible to create a publisher and add a dataWriter to it later on
 */
typedef struct {
    DDS_Publisher dds_publisher;        ///< DDS Publisher used to publish to a topic
    DDS_DataWriter dds_dataWriter;      ///< Datawriter to actually write data to a topic.
    bool has_dataWriter;                ///< Boolean indicating whether the publisher has an associated dataWriter
} publisher_t;

/**
 * @brief Create a new publisher for a topic
 * 
 * This only creates a DDS publisher. The topic is only required when creating the datawriter object.
 * It is adviced to use add_publisher(domain_participant, topic) in dds_lib.h to create a new publisher.
 * 
 * @param domain_participant Handle to a participant that is in same partition as the topic to publish to.
 * 
 * @return A new publisher that can be used to publish to topics in the specified DDS partition.
 */
publisher_t publisher_create_new(const domain_participant_t* domain_participant, const DDS_PublisherQos* pub_qos);

/**
 * @brief Private function that adds a new data writer to a publisher.
 *
 * The data writer is actually dependend on the used topic
 * 
 * @param publisher Handle to the publisher where the datawriter should be added to
 * @param topic Handle to the topic where the new datawriter should be able to publish to
 */
void publisher_dataWriter_create_new(publisher_t* publisher, const DDS_DataWriterQos* dw_qos, const topic_t* topic);

DDS_PublisherQos* get_default_publisher_qos(const domain_participant_t* domain_participant);

/**
 * @brief Frees all resources kept by the publisher
 * 
 * @param publisher Handle to the publisher to delete
 * @param domain_participant Handle to a participant used to create this publisher
 */
void publisher_cleanup(publisher_t* publisher, const domain_participant_t* domain_participant);

#endif