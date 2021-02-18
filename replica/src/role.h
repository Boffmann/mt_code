#ifndef __REPLICA_ROLE_H__
#define __REPLICA_ROLE_H__

typedef enum {
    FOLLOWER,
    CANDIDATE,
    LEADER
} RaftRole;


#endif