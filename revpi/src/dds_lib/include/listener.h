#ifndef __RASPI_DDSLIB_LISTENER_H__
#define __RASPI_DDSLIB_LISTENER_H__

#include "dds_dcps.h"
#include "domain_participant.h"
#include "topic.h"

/**
 * @brief Represents a listener that listens for new messages on a topic.
 * 
 * This structure combines a DDS subscriber, dataReader and listener that together
 * allow to inform about new messages arriving at a topic.
 */
typedef struct {
    DDS_Subscriber dds_subscriber;                  ///< The DDS subscriber used to subscribe to a topic
    DDS_DataReader dds_dataReader;                  ///< DDS DataReader used to read data on a topic
    struct DDS_DataReaderListener *dds_listener;    ///< Listener to call callback when new data arrives at topic.
} listener_t;

/**
 * @brief Create a new listener to listen on a topic.
 * 
 * The actual callback function needs to be specified in accordance to the actual topic to listen to.
 * 
 * @param domain_participant A handle to a participant that is in the same partition as the topic to listen to
 * @param topic Handle to the topic to listen to
 * 
 * @return A new listener that listens for new messages on the specified topic
 */
listener_t listener_create_new(const domain_participant_t* domain_participant, const DDS_SubscriberQos* sub_qos);

void listener_create_dataReader_new(listener_t* listener, const DDS_DataReaderQos* dr_qos, const topic_t* topic);

DDS_SubscriberQos* get_default_subscriber_qos(const domain_participant_t* domain_participant);

/**
 * @brief Frees all resources kept by the listener
 * 
 * @param listener Handle to the listener to delete
 * @param domain_participant Handle to a participant used to create this listener
 */
void listener_cleanup(const listener_t* listener, const domain_participant_t* domain_participant);

#endif