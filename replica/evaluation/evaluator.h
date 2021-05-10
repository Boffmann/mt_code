#ifndef __EVALUATOR_H__
#define __EVALUATOR_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/**
 * Possible reasons why a train must be stopped
 */
enum StoppedReason {
    NONE = 0,                                   // Train must not be stopped or reason unknown
    END_OF_MA = 1,                              // Train reaches and of current MA
    BALISE_NOT_LINKED = 2,                      // A balise was encountered that has not been linked
    BALISE_NOT_WHERE_EXPECTED = 3,              // A balise was encountered at a different position than it was linked
    HAS_NO_STATE = 4                            // The train's state is not accessable
};

extern uint32_t num_missed_election_timeouts;   // Evaluation file for the number of failed leaders
FILE *num_missed_election_timeouts_FILE;        // Evaluation file for the number of missed election timeouts
FILE *time_until_leader_elected_FILE;           // Evaluation file for the time that it takes to elect a new leader
FILE *scenario_evaluation_FILE;                 // Evaluation file for automatic integration testing

/**
 * Initialize the evaluation component
 */
void initialize_evaluator();

/**
 * Notify the evaluation component about a missing heartbeat
 * 
 * @param id Id of the replica that recognized missing heartbeat message
 * @param term the term in which the heartbeat message is missed
 */
void evaluator_registered_election_timeout(const uint32_t id, const uint32_t term);

/**
 * Notify the evaluation component about a started leader election
 */
void evaluator_leader_election_started();

/**
 * Notify the evaluation component about a finished leader election
 * 
 * @param id Id of the new leader
 * @param term The term in which the leader got elected
 */
void evaluator_got_new_leader(const uint8_t id, const uint32_t term);


/**
 * Notify the evaluation component about a started journey
 */
void evaluator_start_new_jouney();

/**
 * Notify the evaluation component about a balise telegram that was received
 * 
 * @param position The current train position
 * @param balise_id Id of the balise whose telegram was received
 */
void evaluator_reached_balise(const double position, int balise_id);

/**
 * Notify the evaluation component that the train must brake
 * 
 * @param stopped_position Position at which the braking decision has been made
 * @param balise_id ID of the balise whose telegram lead to the brake decision. 0 if should brake because end of MA is reached
 * @param reason Reason for braking
 */
void evaluator_train_stopped(const double stopped_position, int balise_id, enum StoppedReason reason);

#endif