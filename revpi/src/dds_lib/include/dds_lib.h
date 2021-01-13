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

/**
 * Sets up the entire DDS domain.
 * This includes the DDS domain, as well as involved DDS domain participants.
 */
domain_participant_t setup_dds_domain(char* partition_name);

topic_t join_topic(domain_participant_t* domain_participant, const TopicType type);

publisher_t add_publisher(const domain_participant_t* domain_participant);



/**
 * Registers a new actor and adds it to the pool of existing actors.
 * Each actor has an address (e.g. it's IP address) and a role.
 * 
 * address: An unique identifier for the actor. Is used to access the actor 
 * role: The role this actor has in the system
 * 
 * returns: A struct representing the new actor
 */
// const ddslib_actor_t register_new_actor(const domain_participant_t* domain_participant, const char* address, const ROLE role) {

/**
 * Unregisters an actor and removes it from the active actors list.
 * 
 * actor: The actor to unregister
 * 
 * returns: Error code
 */
// short unregister_actor(const ddslib_actor_t* actor);

/**
 * Registers an actor to listen on the tasks topic
 * This creates a dds listener and calls the callback function for each new
 * task that is published to the tasks topic and that is addressed to the actor.
 * 
 * actor: The actor to register to the topic
 * on_task_topic_data_received: Function to invoke when new task data is present on the topic.
 * 
 * returns: Error code:
 *      - 0: No error occured. 
 */
// short listen_for_new_tasks(const ddslib_actor_t* actor, task_data_received_t on_task_topic_data_received);

/**
 * Publish a new task for an actor to work on.
 * The task is addressed to a specific worker.
 * 
 * task_data: Data that describes the task.
 * receiver: The actor that should work on this task.
 * 
 * returns: Error code:
 *      - 0: No error occured 
 */
// short publish_new_task(const task_topic_data_t* task_data, const ddslib_actor_t* receiver);

/**
 * Publish a new task for a group of actors to work on.
 * The task is addressed to all workers taking up a particular role.
 * 
 * task_data: Data that describes the task.
 * role: The ROLE of the actors that should receive this task data
 * 
 * returns: Error code
 *      - 0: No error occured
 */
// short publish_new_task(const task_topic_data_t* task_data, const ROLE role);



#endif