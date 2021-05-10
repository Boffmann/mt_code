#ifndef __REVPI_REPLICA_H__
#define __REVPI_REPLICA_H__

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

#include "evaluation/evaluator.h"

#include "dds_dcps.h"

#include "datamodel.h"

#define VOTED_NONE 255      // Used to indicate that the replica has not voted in current term
#define ACTIVE_REPLICAS 3   // Used to specify how many replicas are in the system
#define NUM_SPARES 1        // Used to specify how many spares are in the system

#define NO_ENTRIES_ID 0     // Used to indicate that decision making does not belong to any data but braking curve evaluation
#define HEARTBEAT_ID 1      // Used to indicate that appendEntries message is a heartbeat message

/**
 * Data structure that encodes a replica's decision to a log entry
 */
typedef struct {
    uint32_t replica_id;                // The replica's id that made a decision
    uint32_t term;                      // The term in which this result was generated
    bool should_break;                  // Specifies whether the decision was to brake or continue driving
    enum StoppedReason reason;          // Reason that a train must stop when decided to brake
} replica_result_t;

/**
 * Specifies different roles for the Raft cluster
 */
typedef enum {
    FOLLOWER,                           // A follower processes instructions from leaders
    CANDIDATE,                          // A candidate tries to become the new leader in the system
    LEADER,                             // A leader is responsible for reading inputs and generating a single system output
    SPARE                               // A spare is a hot standby that can be activated by the leader, if needed
} RaftRole;

/**
 * A replica is the main data structure for the raft centric cluster
 */
typedef struct {

    uint8_t ID;                         // An unique identifier for this replica
    RaftRole role;                      // The replica's current raft role
    uint32_t current_term;              // The replica's current term in which this replica is
    uint32_t election_term;             // The term in which the replica candidated to become leader
    uint8_t voted_for;                  // The id for the replica that this replica voted for in current_term

    volatile bool waiting_for_votes;    // Used as the controlling variable for the vote collection thread 

    pthread_mutex_t consensus_mutex;    // Mutex for atomically accessing this replica's fields
    pthread_mutex_t heartbeat_cond_lock;// Mutex for the cond_send_heartbeats condition variable
    pthread_cond_t cond_send_heartbeats;// Condition variable that is used to start and stop heartbeat sending
    pthread_t election_timer_thread;    // The thread in which the election timer is read
    pthread_t request_vote_thread;      // The thread in which requests votes and replies are processed
    pthread_t heartbeat_thread;         // The thread in which the heartbeats are sent by the leader

    struct timespec heartbeat_timeout;  // Frequency for heartbeat sending

    DDS_Duration_t election_timeout;    // Timeout at which a leader is declared dead when heartbeats are missed

} replica_t;

/**
 * This replica's instance for accessing the replicas properties
 * Only valid after calling initialize_replica
 */
replica_t* this_replica;

/**
 * Used for initializing the replica
 * 
 * @param id The id that the replica should have. If greater than ACTIVE_REPLICAS, this replica
 * is considered a spare
 */
void initialize_replica(const uint8_t id);

/**
 * Used to teardown the replica and free all used resources
 */
void teardown_replica();

/**
 * Let this_replica become a follower for the given term
 * 
 * @param term the term in which the replica should become a follower
 */
void become_follower(const uint32_t term);

/**
 * Let this_replica become a spare unit
 */
void become_spare();

/**
 * Let this_replica become a leader
 */
void become_leader();

/**
 * Spread an input across the cluster and collect the decisions.
 * When enough decisions are collected, on_result is called. Else, on_fail is called
 * 
 * @param handle Handle for the input message. Used to dispose the message during commit phase
 * @param on_result function pointer that is invoked when decisions are collected successfully
 * @param on_fail function pointer that is invoked when the cluster failed to process the input
 */
void cluster_process(const uint32_t inputID, const int baliseID, void(*on_result)(const uint32_t inputID, const int baliseID, const replica_result_t* result, const size_t length), void(*on_fail)(void));

#endif