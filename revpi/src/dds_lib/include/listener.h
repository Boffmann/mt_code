#ifndef __RASPI_DDSLIB_LISTENER_H__
#define __RASPI_DDSLIB_LISTENER_H__

#include "dds_dcps.h"
#include "datamodelSacDcps.h"
#include "domain_participant.h"
#include "subscriber.h"
#include "topic.h"

typedef struct {
    RevPiDDS_DecisionDataReader     decision_data_reader;
    RevPiDDS_StateDataReader        state_data_reader;
    RevPiDDS_TasksDataReader        task_data_reader;
} listener_data_t;

/**
 * @brief Represents a listener that listens for new messages on a topic.
 * 
 * This structure combines a DDS subscriber, dataReader and listener that together
 * allow to inform about new messages arriving at a topic.
 */
typedef struct {
    subscriber_t subscriber;
    struct DDS_DataReaderListener *dds_listener;    ///< Listener to call callback when new data arrives at topic.
    listener_data_t* listener_data;                            ///< Pointer to the listener data. It's important to use a listener for one topic only
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

/**
 * @brief Frees all resources kept by the listener
 * 
 * @param listener Handle to the listener to delete
 * @param domain_participant Handle to a participant used to create this listener
 */
void listener_cleanup(listener_t* listener, const domain_participant_t* domain_participant);

#endif