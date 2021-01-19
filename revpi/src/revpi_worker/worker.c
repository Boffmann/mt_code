#include "worker.h"
#include "dds_lib/include/publisher.h"
#include "dds_lib/include/dds_lib.h"

volatile bool running = true;

publisher_t g_decision_publisher;
subscriber_t g_state_subscriber;

void schedule_new_task(const task_t* task_data) {

    state_t state_data;
    decision_t decision_data;
    printf("THIS IS THE TASK DATA CALLBACK\n");

    printf("RECEIVED THIS TASK ID: %d\n", task_data->task_type);

    if (task_data->task_type == SHUTDOWN) {

        running = false;

    } else if (task_data->task_type == SPEED_MONITORING) {

        bool success = state_topic_read(&g_state_subscriber, &state_data);

        if (success) {
            printf("Got the speed: %f\n", state_data.speed);
        } else {
            printf("Could not read speed\n");
        }

        decision_data.sender_id = 1;
        
        if (state_data.speed < 10.0) {
            decision_data.decision = 1;
            decision_topic_publish(&g_decision_publisher, &decision_data);
        } else {
            decision_data.decision = 2;
            decision_topic_publish(&g_decision_publisher, &decision_data);
        }
    }
}


void worker_main(domain_participant_t* domain_participant, topic_t* decision_topic, topic_t* state_topic, topic_t* task_topic) {

        (void) decision_topic;
        (void) task_topic;

        DDS_SubscriberQos* sub_qos = get_default_subscriber_qos(domain_participant);
        listener_t tasks_listener = listener_create_new(domain_participant, sub_qos);
        g_state_subscriber = subscriber_create_new(domain_participant, sub_qos);
        DDS_free(sub_qos);
        DDS_DataReaderQos* dr_qos = dr_qos_copy_from_topic_qos(&tasks_listener.subscriber, task_topic);
        listener_create_dataReader_new(&tasks_listener, dr_qos, task_topic);
        DDS_free(dr_qos);
        DDS_DataReaderQos* dr_qos_state = dr_qos_copy_from_topic_qos(&g_state_subscriber, state_topic);
        subscriber_dataReader_create_new(&g_state_subscriber, dr_qos_state, state_topic);
        DDS_free(dr_qos_state);

        DDS_PublisherQos* pub_qos = get_default_publisher_qos(domain_participant);
        g_decision_publisher = publisher_create_new(domain_participant, pub_qos);
        DDS_free(pub_qos);
        DDS_DataWriterQos* dw_qos = dw_qos_copy_from_topic_qos(&g_decision_publisher, decision_topic);
        publisher_dataWriter_create_new(&g_decision_publisher, dw_qos, decision_topic);
        DDS_free(pub_qos);

        task_topic_listen(&tasks_listener, &schedule_new_task);

        while (running) {
        // for (int i = 0; i < 15; ++i) {
            sleep(2);
            state_t state_data;
            bool success = state_topic_read(&g_state_subscriber, &state_data);

            if (success) {
                printf("Got the speed: %f\n", state_data.speed);
            } else {
                printf("Could not read speed\n");
            }
        }

        listener_cleanup(&tasks_listener, domain_participant);
        publisher_cleanup(&g_decision_publisher, domain_participant);
        subscriber_cleanup(&g_state_subscriber, domain_participant);



}