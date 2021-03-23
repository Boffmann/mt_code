#ifndef __LEADER_ELECTION_H__
#define __LEADER_ELECTION_H__

/**
 * Start the leader election process
 */
void run_leader_election();

/**
 * Handle vote requests and vote request replies.
 * This method is intendet to be used as an anchor point for a background thread
 */
void *receive_vote_requests();

#endif