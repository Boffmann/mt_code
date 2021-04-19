#ifndef __EVALUATOR_H__
#define __EVALUATOR_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <pthread.h>

#define MEASURE_NUM_ELECTION_TIMEOUTS false

pthread_mutex_t fileWrite_mutex;

enum StoppedReason {
    NONE = 0,
    END_OF_MA = 1,
    BALISE_NOT_LINKED = 2,
    BALISE_NOT_WHERE_EXPECTED = 3,
    HAS_NO_STATE = 4
};

extern uint32_t num_missed_election_timeouts;
FILE *num_missed_election_timeouts_FILE;
FILE *time_until_leader_elected_FILE;
FILE *scenario_evaluation_FILE;
FILE *message_send_FILE;
FILE *message_received_FILE;

void initialize_evaluator();
void evaluator_registered_election_timeout(const uint32_t id, const uint32_t term);
void evaluator_leader_election_started();
void evaluator_got_new_leader(const uint8_t id, const uint32_t term);

void evaluator_start_new_jouney();
void evaluator_reached_balise(const double position, int balise_id);
void evaluator_train_stopped(const double stopped_position, int balise_id, enum StoppedReason reason);

void evaluator_register_message_send(const uint8_t senderID, char* topic, bool is_leader);
void evaluator_register_message_received(const uint8_t senderID, char* topic, bool is_leader);

#endif