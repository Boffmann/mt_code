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

    printf("RECEIVED THIS TASK ID: %d\n", task_data->task_type);
}


DDS_TopicQos* create_topic_qos(const domain_participant_t* domain_participant) {
    DDS_TopicQos* topic_qos = get_default_domain_topic_qos(domain_participant);
    topic_qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    topic_qos->durability.kind = DDS_VOLATILE_DURABILITY_QOS;

    return topic_qos;
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

    domain_participant_t domain_participant = domain_participant_create_new("Test_Partition");

    DDS_TopicQos* topic_qos = create_topic_qos(&domain_participant);

    topic_t tasks_topic = topic_join(&domain_participant, topic_qos, TASKS);

    DDS_free(topic_qos);

    if (mode == TASK_PUBLISHER) {

        printf("Publishing Mode\n");

        // publisher_t tasks_publisher = add_publisher(&domain_participant, &tasks_topic);
        DDS_PublisherQos* tasks_publisher_qos = get_default_publisher_qos(&domain_participant);
        publisher_t tasks_publisher = publisher_create_new(&domain_participant, tasks_publisher_qos);
        DDS_free(tasks_publisher_qos);
        DDS_DataWriterQos* tasks_dw_qos = dw_qos_copy_from_topic_qos(&tasks_publisher, &tasks_topic);
        publisher_dataWriter_create_new(&tasks_publisher, tasks_dw_qos, &tasks_topic);
        DDS_free(tasks_dw_qos);

        for (int i = 0; i < 10; ++i) {

            task_t task_data;
            task_data.task_type = SPEED_MONITORING;
            task_topic_publish(&tasks_publisher, &task_data);
            printf("Published: %d\n", i);

            sleep(2);            
        }

        publisher_cleanup(&tasks_publisher, &domain_participant);

    } else {

        printf("Listening Mode\n");

        DDS_SubscriberQos* sub_qos = get_default_subscriber_qos(&domain_participant);
        listener_t tasks_listener = listener_create_new(&domain_participant, sub_qos);
        DDS_free(sub_qos);
        DDS_DataReaderQos* dr_qos = dr_qos_copy_from_topic_qos(&tasks_listener, &tasks_topic);
        listener_create_dataReader_new(&tasks_listener, dr_qos, &tasks_topic);
        DDS_free(dr_qos);


        task_topic_listen(&tasks_listener, &schedule_new_task);

        for (int i = 0; i < 15; ++i) {
            sleep(2);
        }

        listener_cleanup(&tasks_listener, &domain_participant);

    }

    topic_leave(&tasks_topic, &domain_participant);

    domain_participant_delete(&domain_participant);

}