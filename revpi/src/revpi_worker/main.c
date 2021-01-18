#include "dds_lib/include/dds_lib.h"
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "DIO.h"
#include "worker.h"

// TODO There also is a find_topic_function

typedef enum {

    TASK_WORKER,
    TASK_PUBLISHER

} RunningMode;

void new_decision(const decision_t* decision_data) {
    printf("A new decision1!!!\n");
    printf("Deciosion is: %ld\n", decision_data->decision_id);
}

DDS_TopicQos* create_topic_qos(const domain_participant_t* domain_participant) {
    DDS_TopicQos* topic_qos = get_default_domain_topic_qos(domain_participant);
    topic_qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    topic_qos->durability.kind = DDS_PERSISTENT_DURABILITY_QOS;

    return topic_qos;
}

int main(int argc, char *argv[]) {

    RunningMode mode;
    int opt;

    if (argc < 2) {
        printf("Usage: %s [-wp]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "wp")) != -1) {
        switch (opt) {
            case 'w': mode = TASK_WORKER; break;
            case 'p': mode = TASK_PUBLISHER; break;
            default:
                printf("Usage: %s [-wp]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    domain_participant_t domain_participant = domain_participant_create_new("Test_Partition");

    DDS_TopicQos* topic_qos = create_topic_qos(&domain_participant);

    topic_t decision_topic = topic_join(&domain_participant, topic_qos, DECISIONS);
    topic_t state_topic = topic_join(&domain_participant, topic_qos, STATE);
    topic_t tasks_topic = topic_join(&domain_participant, topic_qos, TASKS);

    DDS_free(topic_qos);

    if (mode == TASK_PUBLISHER) {

        printf("Publishing Mode\n");

        DDS_SubscriberQos* sub_qos = get_default_subscriber_qos(&domain_participant);
        listener_t decision_listener = listener_create_new(&domain_participant, sub_qos);
        DDS_free(sub_qos);
        DDS_DataReaderQos* dr_qos = dr_qos_copy_from_topic_qos(&decision_listener.subscriber, &decision_topic);
        listener_create_dataReader_new(&decision_listener, dr_qos, &decision_topic);
        DDS_free(dr_qos);

        DDS_PublisherQos* tasks_publisher_qos = get_default_publisher_qos(&domain_participant);
        publisher_t tasks_publisher = publisher_create_new(&domain_participant, tasks_publisher_qos);
        publisher_t state_publisher = publisher_create_new(&domain_participant, tasks_publisher_qos);
        DDS_free(tasks_publisher_qos);
        DDS_DataWriterQos* tasks_dw_qos = dw_qos_copy_from_topic_qos(&tasks_publisher, &tasks_topic);
        publisher_dataWriter_create_new(&tasks_publisher, tasks_dw_qos, &tasks_topic);
        DDS_free(tasks_dw_qos);
        DDS_DataWriterQos* state_dw_qos = dw_qos_copy_from_topic_qos(&tasks_publisher, &state_topic);
        publisher_dataWriter_create_new(&state_publisher, state_dw_qos, &state_topic);
        DDS_free(state_dw_qos);

        dds_state_instance_t state_instance = state_topic_create_new_instance(&state_publisher);

        decision_topic_listen(&decision_listener, &new_decision);

        task_t task_data;
        // state_t state_data;
        for (int i = 0; i < 10; ++i) {

            task_data.task_type = SPEED_MONITORING;
            task_topic_publish(&tasks_publisher, &task_data);
            // state_data.timestamp = 3;
            // state_data.speed = i * 2.0;
            state_instance.message->timestamp = 3;
            state_instance.message->speed = i * 2.0;
            state_topic_publish(&state_publisher, &state_instance);
            printf("Published: %f\n", i *2.0);

            sleep(2);            
        }

        task_data.task_type = SHUTDOWN;
        task_topic_publish(&tasks_publisher, &task_data);

        state_topic_dispose_instance(&state_publisher, &state_instance);
        publisher_cleanup(&tasks_publisher, &domain_participant);
        publisher_cleanup(&state_publisher, &domain_participant);
        listener_cleanup(&decision_listener, &domain_participant);

    } else {

        worker_main(&domain_participant, &decision_topic, &state_topic, &tasks_topic);

    }

    topic_leave(&tasks_topic, &domain_participant);
    topic_leave(&state_topic, &domain_participant);
    topic_leave(&decision_topic, &domain_participant);

    domain_participant_delete(&domain_participant);

}