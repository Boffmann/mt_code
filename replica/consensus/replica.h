#ifndef __REVPI_REPLICA_H__
#define __REVPI_REPLICA_H__

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/time.h>

#include "dds_dcps.h"

#include "datamodel.h"

#define VOTED_NONE 255
#define ACTIVE_REPLICAS 3
#define LOG_PUFFER 5

enum {
    WON_ELECTION        = (1 << 0),
    LOST_ELECTION       = (1 << 1),
    TIMEOUT             = (1 << 2)
};

struct CollectVotesParams {
    uint32_t started_term;
    uint8_t votes_received;
};

typedef struct {
    uint32_t id;
    uint32_t term;
} log_entry_t;

typedef struct {
    uint32_t replica_id;
    uint32_t term;          // The term in which this result was generated
} replica_result_t;

typedef enum {
    FOLLOWER,
    CANDIDATE,
    LEADER
} RaftRole;

typedef struct {

    uint8_t ID;
    RaftRole role;
    uint32_t current_term;
    uint32_t election_term;     // The term in which the replica candidated to become leader
    uint8_t voted_for;

    pthread_mutex_t consensus_mutex;
    pthread_t election_timer_thread;
    pthread_t request_vote_thread;

    struct sigaction heartbeat_action;
    struct itimerval heartbeat_timer;
    DDS_Duration_t election_timeout;

    // log_entry_t log[LOG_PUFFER];
    // size_t log_tail;

} replica_t;

replica_t* this_replica;

void initialize_replica(const uint8_t id);
void teardown_replica();

void become_follower(const uint32_t term);
void become_leader();

void cluster_process(RevPiDDS_Input* handle, void(*on_result)(RevPiDDS_Input* handle, const replica_result_t* result, const size_t length), void(*on_fail)(void));

#endif