#ifndef __EVALUATOR_H__
#define __EVALUATOR_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define MEASURE_NUM_ELECTION_TIMEOUTS true

extern uint32_t num_missed_election_timeouts;
FILE *num_missed_election_timeouts_FILE;
FILE *time_until_leader_elected_FILE;

void initialize_evaluator();
void evaluator_registered_election_timeout(const uint32_t id, const uint32_t term);
void evaluator_leader_election_started();
void evaluator_got_new_leader(const uint8_t id, const uint32_t term);

// void eval_scenario_write_ma()

#endif