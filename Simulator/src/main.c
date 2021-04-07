#include <stdbool.h>
#include <sys/time.h>
#include <stdlib.h>

#include "dds_dcps.h"
#include "CheckStatus.h"
#include "DDSEntitiesCreator.h"

#include "simulator.h"
#include "scenario.h"

#define TRAIN_SPEED 1

scenario_t scenario;

void setup() {

    domainParticipant = createParticipant("Test_Partition");

    // simulator_init();
    create_scenario_from("../Scenarios/scenario1.json", &scenario);

    printf("The scenario has %ld balises and %d linked\n", scenario.balises.used, scenario.num_linked_balises);

    simulator_init();

}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    srand(time(NULL));

    setup();

    balise_array_t* linked_balises = scenario_get_linked_balises(&scenario);

    printf("The scenario has %ld linked balises\n", linked_balises->used);
    printf("The first linked balises position: %d\n", linked_balises->array[0].position);


    sleep(1);
    send_movement_authority(&scenario);
    send_linking_information(linked_balises);

    struct timeval time_started;
    gettimeofday(&time_started, NULL);
    unsigned long long started_microseconds = time_started.tv_sec * 1000000L + time_started.tv_usec;

    double train_position = 0.0;
    size_t next_balise_index = 0;

    while (true) {
        struct timeval time_now;
        gettimeofday(&time_now, NULL);
        unsigned long long elapsed_time = (time_now.tv_sec * 1000000L + time_now.tv_usec) - started_microseconds;

        train_position = TRAIN_SPEED * ((double) elapsed_time / 1000000.0);

        printf("Train position: %lf\n", train_position);

        if (next_balise_index >= scenario.balises.used) {
            break;
        }

        balise_t *next_balise = &scenario.balises.array[next_balise_index];
        
        if (train_position > next_balise->position) {
            send_balise(next_balise);
            next_balise_index++;
        }

        sleep(1);

    }

    free(linked_balises);
    scenario_cleanup(&scenario);

    sleep(1);
    send_terminate();

}
