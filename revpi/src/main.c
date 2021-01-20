#include "DDSEntitiesCreator.h"
#include "StateTopic.h"
#include "TaskTopic.h"
#include "DecisionTopic.h"
#include "CheckStatus.h"

DDS_DataReader state_DataReader = DDS_OBJECT_NIL;
struct StateMessage state_message;

void new_decision_callback(const decision_data_t* decision_data) {

    (void) state_DataReader;
    printf("Callback triggered with decision data: %ld\n", decision_data->decision);
}

void new_task_callback(const task_data_t* task_data) {
    printf("Callback triggered for new task data. The task id is: %ld\n", task_data->taskType);

    state_data_t state_data;

    stateTopic_read(state_DataReader, &state_message, &state_data);

    printf("Speed is: %f\n", state_data.speed);

}

typedef enum {
    PUBLISHER,
    WORKER
} RunningMode;

int main(int argc, char *argv[]) {

    DDS_DomainParticipant domainParticipant;
    DDS_Topic stateTopic, decisionTopic, taskTopic;
    RunningMode mode;
    int opt;

    if (argc < 2) {
        printf("Usage: %s [-wp]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "wp")) != -1) {
        switch (opt) {
            case 'p': mode = PUBLISHER; break;
            case 'w': mode = WORKER; break;
            default:
                printf("Usage: %s [-wp]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    domainParticipant = createParticipant("Test_Partition");
    stateTopic = stateTopic_create(domainParticipant, "State_Topic");
    taskTopic = stateTopic_create(domainParticipant, "Task_Topic");
    decisionTopic = stateTopic_create(domainParticipant, "Decision_Topic");

    if (mode == PUBLISHER) {

        DDS_PublisherQos *task_publisherQos, *state_publisherQos;
        DDS_DataWriterQos *task_dataWriterQos, *state_dataWriterQos;
        DDS_SubscriberQos *decision_subscriberQos;
        DDS_DataReaderQos *decision_dataReaderQos;

        DDS_Publisher task_Publisher;
        DDS_DataWriter task_DataWriter;
        DDS_Publisher state_Publisher;
        DDS_DataWriter state_DataWriter;

        DDS_Subscriber decision_Subscriber;
        DDS_DataReader decision_DataReader;
        struct DDS_DataReaderListener *decision_listener;

        task_publisherQos = taskTopic_getPublisherQos(domainParticipant);
        state_publisherQos = stateTopic_getPublisherQos(domainParticipant);
        task_Publisher = createPublisher(domainParticipant, task_publisherQos);
        state_Publisher = createPublisher(domainParticipant, state_publisherQos);
        DDS_free(task_publisherQos);
        DDS_free(state_publisherQos);

        task_dataWriterQos = taskTopic_getDataWriterQos(task_Publisher, taskTopic);
        state_dataWriterQos = stateTopic_getDataWriterQos(state_Publisher, stateTopic);
        task_DataWriter = createDataWriter(task_Publisher, taskTopic, task_dataWriterQos);
        state_DataWriter = createDataWriter(state_Publisher, stateTopic, state_dataWriterQos);
        DDS_free(task_dataWriterQos);
        DDS_free(state_dataWriterQos);

        decision_subscriberQos = decisionTopic_getSubscriberQos(domainParticipant);
        decision_Subscriber = createSubscriber(domainParticipant, decision_subscriberQos);
        DDS_free(decision_subscriberQos);
        decision_dataReaderQos = decisionTopic_getDataReaderQos(decision_Subscriber, decisionTopic);
        decision_DataReader = createDataReader(decision_Subscriber, decisionTopic, decision_dataReaderQos);
        DDS_free(decision_dataReaderQos);

        decision_listener = createDataReaderListener();
        decisionTopic_registerListener(decision_listener, decision_DataReader, &new_decision_callback);

        state_data_t state_data;
        task_data_t task_data;
        for (int i = 0; i < 10; ++i) {
            state_data.timestamp = 1;
            state_data.speed = i * 2.0;
            stateTopic_write(state_DataWriter, &state_data);
            task_data.taskType = SPEED_MONITORING;
            taskTopic_write(task_DataWriter, &task_data);

            sleep(5);
        }


        deleteDataReaderListener(decision_listener);
        deleteDataWriter(state_Publisher, state_DataWriter);
        deleteDataWriter(task_Publisher, task_DataWriter);
        deletePublisher(domainParticipant, state_Publisher);
        deletePublisher(domainParticipant, task_Publisher);

        deleteDataReader(decision_Subscriber, decision_DataReader);
        deleteSubscriber(domainParticipant, decision_Subscriber);

    } else if (mode == WORKER) {

        DDS_SubscriberQos *state_subscriberQos, *task_subscriberQos;
        DDS_DataReaderQos *state_dataReaderQos, *task_dataReaderQos;
        DDS_PublisherQos *decision_publisherQos;
        DDS_DataWriterQos *decision_dataWriterQos;
        DDS_Subscriber state_Subscriber, task_Subscriber;
        DDS_DataReader task_DataReader;
        DDS_Publisher decision_Publisher;
        DDS_DataWriter decision_DataWriter;
        struct DDS_DataReaderListener *task_listener;

        state_subscriberQos = stateTopic_getSubscriberQos(domainParticipant);
        task_subscriberQos = taskTopic_getSubscriberQos(domainParticipant);
        state_Subscriber = createSubscriber(domainParticipant, state_subscriberQos);
        task_Subscriber = createSubscriber(domainParticipant, task_subscriberQos);
        DDS_free(state_subscriberQos);
        DDS_free(task_subscriberQos);

        state_dataReaderQos = stateTopic_getDataReaderQos(state_Subscriber, stateTopic);
        task_dataReaderQos = stateTopic_getDataReaderQos(task_Subscriber, taskTopic);
        state_DataReader = createDataReader(state_Subscriber, stateTopic, state_dataReaderQos);
        task_DataReader = createDataReader(task_Subscriber, taskTopic, task_dataReaderQos);
        DDS_free(state_dataReaderQos);
        DDS_free(task_dataReaderQos);

        decision_publisherQos = decisionTopic_getPublisherQos(domainParticipant);
        decision_Publisher = createPublisher(domainParticipant, decision_publisherQos);
        DDS_free(decision_publisherQos);

        decision_dataWriterQos = decisionTopic_getDataWriterQos(decision_Publisher, decisionTopic);
        decision_DataWriter = createDataWriter(decision_Publisher, decisionTopic, decision_dataWriterQos);
        DDS_free(decision_dataWriterQos);

        stateTopic_newMessage(&state_message);

        task_listener = createDataReaderListener();
        taskTopic_registerListener(task_listener, task_DataReader, &new_task_callback);

        for (int i = 0; i < 10; ++i) {
            sleep(5);
        }

        stateTopic_freeMessage(&state_message);
        deleteDataReaderListener(task_listener);
        deleteDataReader(state_Subscriber, state_DataReader);
        deleteDataReader(task_Subscriber, task_DataReader);
        deleteDataWriter(decision_Publisher, decision_DataWriter);
        deleteSubscriber(domainParticipant, state_Subscriber);
        deleteSubscriber(domainParticipant, task_Subscriber);
        deletePublisher(domainParticipant, decision_Publisher);

    }

//cleanup:
    deleteTopic(domainParticipant, stateTopic);
    deleteTopic(domainParticipant, taskTopic);
    deleteTopic(domainParticipant, decisionTopic);
    deleteParticipant(domainParticipant);

    return 0;
}