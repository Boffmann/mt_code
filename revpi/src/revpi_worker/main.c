#include "dds_lib/include/dds_lib.h"
#include "dds_lib/include/task_topic.h"
#include <stdio.h>
#include <unistd.h>

typedef enum {

    TASK_LISTENER,
    TASK_PUBLISHER

} RunningMode;

void schedule_new_task(const task_t* task_data) {
    printf("THIS IS THE TASK DATA CALLBACK\n");

    printf("RECEIVED THIS TASK ID: %ld\n", task_data->task_id);
}

int main(int argc, char *argv[]) {

    RunningMode mode;
    int opt;

    if (argc < 2) {
        printf("Usage: %s [-lp]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "lp")) != -1) {
        switch (opt) {
            case 'l': mode = TASK_LISTENER; break;
            case 'p': mode = TASK_PUBLISHER; break;
            default:
                printf("Usage: %s [-lp]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    domain_participant_t domain_participant = setup_dds_domain("Test_Partition");

    // topic_t actors_topic = join_topic(&domain_participant, ACTORS); 
    topic_t tasks_topic = join_topic(&domain_participant, TASKS);


    if (mode == TASK_PUBLISHER) {

        printf("Publishing Mode\n");

        publisher_t tasks_publisher = add_publisher(&domain_participant, &tasks_topic);

        for (int i = 0; i < 10; ++i) {

            task_t task_data;
            task_data.message = "Some task data";
            task_data.task_id = i;
            task_topic_publish(&tasks_publisher, &task_data);
            printf("Published: %d\n", i);

            sleep(2);            
        }

        publisher_cleanup(&tasks_publisher, &domain_participant);

    } else {

        printf("Listening Mode\n");

        listener_t tasks_listener = add_listener(&domain_participant, &tasks_topic);

        task_topic_listen(&tasks_listener, &schedule_new_task);

        for (int i = 0; i < 15; ++i) {
            sleep(2);
        }

        listener_cleanup(&tasks_listener, &domain_participant);

    }

    topic_leave(&tasks_topic, &domain_participant);

    domain_participant_delete(&domain_participant);

}