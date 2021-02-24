#ifndef __BACKGROUND_TASKS_H__
#define __BACKGROUND_TASKS_H__

void *run_leader_election_thread();
void *collect_votes();
void *receive_vote_requests();

#endif