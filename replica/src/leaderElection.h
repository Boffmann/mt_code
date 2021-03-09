#ifndef __LEADER_ELECTION_H__
#define __LEADER_ELECTION_H__

void run_leader_election();
void *collect_votes();
void *receive_vote_requests();

#endif