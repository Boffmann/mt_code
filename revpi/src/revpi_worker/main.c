#include "dds_lib/include/task_data.h"
#include <stdio.h>
#include <unistd.h>


void schedule_new_task(struct Task_Data* task_data) {

    printf("This is a call to the callback function\n");
    printf("The received task ID is: %d" ,task_data->task_ID);

    return;

}

// int main(int argc, char *argv[]) {
void main() {

    create_participant("listener_example");

    listen_for_task_data(&schedule_new_task);

    while(1) {

        sleep(1.0);
        struct Task_Data task_data;
        task_data.task_ID = 123;
        publish_task_data(&task_data);

    }

}