#ifndef __REVPI_REPLICA_H__
#define __REVPI_REPLICA_H__

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/time.h>

#define VOTED_NONE 255
#define ACTIVE_REPLICAS 3

enum {
    WON_ELECTION        = (1 << 0),
    LOST_ELECTION       = (1 << 1),
    TIMEOUT             = (1 << 2)
};

struct CollectVotesParams {
    uint32_t started_term;
    uint8_t votes_received;
};

typedef enum {
    FOLLOWER,
    CANDIDATE,
    LEADER
} RaftRole;

typedef struct {

    uint8_t ID;
    RaftRole role;
    uint32_t current_term;
    uint8_t voted_for;

    pthread_mutex_t consensus_mutex;
    pthread_t election_timer_thread;
    pthread_t request_vote_thread;

    struct sigaction heartbeat_action;
    struct itimerval heartbeat_timer;

} replica_t;

replica_t* this_replica;

void initialize_replica(const uint8_t id);
void teardown_replica();

void become_follower(replica_t* replica);
void become_leader(replica_t* replica);

#endif