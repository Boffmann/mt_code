#include "evaluator.h"

#include <sys/time.h>

uint32_t num_missed_election_timeouts = 0;

struct timeval time_leader_election_started;
bool got_leader = true;
uint32_t leader_election_restarts = 0;


void initialize_evaluator() {
    pthread_mutex_init(&fileWrite_mutex, NULL);
}

void evaluator_registered_election_timeout(const uint32_t id, const uint32_t term) {
    num_missed_election_timeouts++;
    num_missed_election_timeouts_FILE = fopen("missed_election_timeouts.csv", "a");

    if (num_missed_election_timeouts_FILE==NULL) {
        printf("Error opening the file for election timeout missed\n");
    } else {
        fprintf(num_missed_election_timeouts_FILE, "%d;%d;%d;\n", id, num_missed_election_timeouts, term);
    }

    fclose(num_missed_election_timeouts_FILE);
}

void evaluator_leader_election_started() {
    if (got_leader) {
        got_leader = false;
        leader_election_restarts = 0;
        gettimeofday(&time_leader_election_started, NULL);
    } else {
        leader_election_restarts++;
    }
}

void evaluator_got_new_leader(const uint8_t id, const uint32_t term) {

    time_until_leader_elected_FILE = fopen("leader_election_duration.csv", "a");
    got_leader = true;

    if (time_until_leader_elected_FILE==NULL) {
        printf("Error opening the file for election timeout missed\n");
    } else {

        struct timeval time_leader_election_ended;
        gettimeofday(&time_leader_election_ended, NULL);

        unsigned long long start_microseconds = time_leader_election_started.tv_sec * 1000000LL + time_leader_election_started.tv_usec;
        unsigned long long end_microseconds = time_leader_election_ended.tv_sec * 1000000LL + time_leader_election_ended.tv_usec;

        fprintf(time_until_leader_elected_FILE, "%d;%d;%lld;%lld;%lld;%d;\n", id, term, start_microseconds, end_microseconds, end_microseconds - start_microseconds, leader_election_restarts);
    }

    fclose(time_until_leader_elected_FILE);
}

const char* reason_to_string(enum StoppedReason reason) {

    switch(reason) {
        case END_OF_MA:
            return "Reached End of MA";
            break;
        case BALISE_NOT_LINKED:
            return "Balise Not Linked";
            break;
        case BALISE_NOT_WHERE_EXPECTED:
            return "Balise Not Where Expected";
            break;
        case HAS_NO_STATE:
            return "Has No State";
            break;
        default:
        return "None";
    }
}

void evaluator_start_new_jouney() {
    scenario_evaluation_FILE = fopen("scenario_evaluation.yml", "w");

    fprintf(scenario_evaluation_FILE, "%s;%s;%s;%s\n", "Position", "Action", "Balise Number", "Reason");

    fclose(scenario_evaluation_FILE);

}

void evaluator_train_stopped(const double stopped_position, int balise_id, enum StoppedReason reason) {
    scenario_evaluation_FILE = fopen("scenario_evaluation.yml", "a");

    if (scenario_evaluation_FILE == NULL) {
        printf("Could not open file for scenario evaluation");
    } else {

        fprintf(scenario_evaluation_FILE, "%lf;%s;%d;%s\n", stopped_position, "Stopped", balise_id, reason_to_string(reason));
    }

    fclose(scenario_evaluation_FILE);
}

void evaluator_reached_balise(const double position, int balise_id) {
    scenario_evaluation_FILE = fopen("scenario_evaluation.yml", "a");

    if (scenario_evaluation_FILE == NULL) {
        printf("Could not open file for scenario evaluation");
    } else {
        fprintf(scenario_evaluation_FILE, "%lf;%s;%d;%s\n", position, "Reached Balise", balise_id, "");
    }

    fclose(scenario_evaluation_FILE);

}

void evaluator_register_message_send(const uint8_t senderID, char* topic, bool is_leader) {

    message_send_FILE = fopen("message_send.csv", "a");

    if (message_send_FILE==NULL) {
        printf("Error opening the file for message send\n");
    } else {

        struct timeval time_now;
        gettimeofday(&time_now, NULL);

        unsigned long long time_now_microseconds = time_now.tv_sec * 1000000LL + time_now.tv_usec;

        printf("Writing into register file\n");
        fprintf(message_send_FILE, "%lld;%d;%s;%d\n", time_now_microseconds, senderID, topic, is_leader);
    }

    fclose(message_send_FILE);

}

void evaluator_register_message_received(const uint8_t senderID, char* topic, bool is_leader) {
    // (void) senderID;
    // (void) topic;
    // (void) is_leader;
    pthread_mutex_lock(&fileWrite_mutex);

    message_received_FILE = fopen("message_received.csv", "a");

    if (message_received_FILE==NULL) {
        printf("Error opening the file for message received\n");
    } else {

        struct timeval time_now;
        gettimeofday(&time_now, NULL);

        unsigned long long time_now_microseconds = time_now.tv_sec * 1000000LL + time_now.tv_usec;

        printf("Writing into received file\n");
        fprintf(message_received_FILE, "%lld;%d;%s;%d\n", time_now_microseconds, senderID, topic, is_leader);
    }

    fclose(message_received_FILE);
    pthread_mutex_unlock(&fileWrite_mutex);


}