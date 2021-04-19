#include <stdbool.h>
#include <sys/time.h>
#include <stdlib.h>

#include "dds_dcps.h"
#include "CheckStatus.h"
#include "DDSEntitiesCreator.h"

#include "simulator.h"
#include "scenario.h"

#include "train.h"

#define TRAIN_SPEED 1

scenario_t scenario;

void setup(char* filePath) {


    // simulator_init();
    bool success = create_scenario_from(filePath, &scenario);

    if (!success) {
        printf("File not found %s\n", filePath);
        exit(1);
    }

    printf("The scenario has %ld balises and %d linked\n", scenario.balises.used, scenario.num_linked_balises);


}

int main(int argc, char *argv[]) {

    domainParticipant = createParticipant("Test_Partition");
    simulator_init();

    if (argc < 2) {
        printf("Should send a terminate command to replicas? y [N]");

        int input = getchar();

        if (input == 'y' || input == 'Y') {
            send_terminate();
        }

        exit(0);
    }

    char *filePath = argv[1];

    printf("%s\n", filePath);

    srand(time(NULL));

    setup(filePath);

    sleep(1);
    send_movement_authority(&scenario);

    balise_array_t* linked_balises = scenario_get_linked_balises(&scenario);
    printf("The scenario has %ld linked balises\n", linked_balises->used);

    send_linking_information(linked_balises);
    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 500000000;
    nanosleep(&timeout, &timeout);

    // struct timeval time_started;
    // gettimeofday(&time_started, NULL);
    // unsigned long long started_microseconds = time_started.tv_sec * 1000000L + time_started.tv_usec;

    // double train_position = 0.0;
    size_t next_balise_index = 0;

    timeout.tv_sec = 0;
    timeout.tv_nsec = 100000000;
    while (true) {

        nanosleep(&timeout, &timeout);
        // struct timeval time_now;
        // gettimeofday(&time_now, NULL);
        // unsigned long long elapsed_time = (time_now.tv_sec * 1000000L + time_now.tv_usec) - started_microseconds;

        // train_position = TRAIN_SPEED * ((double) elapsed_time / 1000000.0);

        train_state_t train_state;
        bool has_state = get_train_state(&train_state);

        if (has_state) {

            printf("Train position: %lf\n", train_state.position.position);

            if (!train_state.is_driving) {
                break;
            }

            if (train_state.position.max_position > scenario.movement_authority.end_position) {
                break;
            }

            if (next_balise_index < scenario.balises.used) {

                balise_t *next_balise = &scenario.balises.array[next_balise_index];

                if (train_state.position.max_position > next_balise->position) {
                    send_balise(next_balise);
                    next_balise_index++;
                }
            }

        }

    }

    free(linked_balises);
    scenario_cleanup(&scenario);

}
