#ifndef __RASPI_DDSLIB_ACTOR_H__
#define __RASPI_DDSLIB_ACTOR_H__

#include "dds_dcps.h"

/**
 * @brief This represents the participating actor's role in the system.
 * 
 * An actor's role is mainly used during testing and showcasing.
 * In the final product most actors will be WORKERS, and one (the main PC)
 * is MAIN
 * 
 */
typedef enum {

    WORKER,     ///< Worker Actors are used for redundant computations of tasks
    MAIN        ///< Main Actors do the main work and rely on WORKERS for redundant computation/decision making

} ActorRole;


/**
 * @brief 
 */
typedef struct {
    // The actors address / identifier
    char* address;
    // The actors role
    ActorRole role;

} actor_t;

/**
 * Adds a new actor to the actor topic in order to be accessable by other actors in the system.
 * 
 * actor: The actor to add to the topic.
 */
void ddslib_actor_publish_add(const actor_t* actor);

/**
 * Removes an actor from the global actor topic.
 * 
 * actor: The actor to remove from the topic.
 */
void ddslib_actor_publish_remove(const actor_t* actor);

#endif