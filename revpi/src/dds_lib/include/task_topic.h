#ifndef __RASPI_DDSLIB_TASKTOPIC_H__
#define __RASPI_DDSLIB_TASKTOPIC_H__

#include "dds_dcps.h"
#include "topic.h"
#include "domain_participant.h"
#include "listener.h"
#include "publisher.h"

// Type forward declaration
struct task_listener_data_t;

/**
 * @brief Represents a topic that stores task information.
 * 
 * The task information is used by WORKERS.
 * The WORKER uses the task information to process a specific task and
 * publish its decision
 */
typedef struct {
    long task_id;   ///< TODO
    char* message;
} task_t;

/// Typedef for function pointer signature that is used for a callback when new task data arrives
typedef void (*on_task_data_available_t)(const task_t* task_data);

/// The actual callback to call when new task data arrives
extern on_task_data_available_t on_task_data_available_callback;

/**
 * @brief Create a new TASK topic
 * 
 * @param domain_participant Handle to participant that is in same partition as this topic
 * 
 * @return A new task topic
 */ 
topic_t tasks_topic_create(const domain_participant_t* domain_participant, const DDS_TopicQos* topic_qos);

/**
 * @brief Publish new task data to the task topic
 * 
 * @param publisher Handle to a publisher that can publish to a task topic
 * @param task_data Handle to the actual data to publish
 */
void task_topic_publish(const publisher_t* publisher, const task_t* task_data);

/**
 * @brief Register a listener to a task topic
 * 
 * This registers a listener to the task topic. When the listener recognizes new data on the task
 * topic, a specified callback function is called with the new task data that is on the topic.
 * 
 * @param listener Handle to the listener to use to listen on the task topic
 * @param callback Callback function to call when new task_data is available
 */
void task_topic_listen(listener_t* listener, on_task_data_available_t callback);

#endif