#include "DDSEntitiesCreator.h"
#include "StateTopic.h"
#include "CheckStatus.h"


typedef enum {
    PUBLISHER,
    SUBSCRIBER
} RunningMode;

int main(int argc, char *argv[]) {

    DDS_DomainParticipant domainParticipant;
    DDS_Topic stateTopic;
    RunningMode mode;
    int opt;

    if (argc < 2) {
        printf("Usage: %s [-sp]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "sp")) != -1) {
        switch (opt) {
            case 's': mode = SUBSCRIBER; break;
            case 'p': mode = PUBLISHER; break;
            default:
                printf("Usage: %s [-sp]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    domainParticipant = createParticipant("Test_Partition");
    stateTopic = stateTopic_create(domainParticipant, "State_Topic");

    if (mode == PUBLISHER) {

        DDS_PublisherQos* publisherQos;
        DDS_DataWriterQos* dataWriterQos;
        DDS_Publisher state_Publisher;
        DDS_DataWriter state_DataWriter;

        publisherQos = stateTopic_getPublisherQos(domainParticipant);
        state_Publisher = createPublisher(domainParticipant, publisherQos);
        DDS_free(publisherQos);

        dataWriterQos = stateTopic_getDataWriterQos(state_Publisher, stateTopic);
        state_DataWriter = createDataWriter(state_Publisher, stateTopic, dataWriterQos);
        DDS_free(dataWriterQos);

        state_data_t data;
        for (int i = 0; i < 10; ++i) {
            data.timestamp = 1;
            data.speed = i * 2.0;
            stateTopic_write(state_DataWriter, &data);
            sleep(5);
        }

        deleteDataWriter(state_Publisher, state_DataWriter);
        deletePublisher(domainParticipant, state_Publisher);

    } else if (mode == SUBSCRIBER) {

        DDS_SubscriberQos* subscriberQos;
        DDS_DataReaderQos* dataReaderQos;
        DDS_Subscriber state_Subscriber;
        DDS_DataReader state_DataReader;
        struct StateMessage state_message;

        subscriberQos = stateTopic_getSubscriberQos(domainParticipant);
        state_Subscriber = createSubscriber(domainParticipant, subscriberQos);
        DDS_free(subscriberQos);

        dataReaderQos = stateTopic_getDataReaderQos(state_Subscriber, stateTopic);
        state_DataReader = createDataReader(state_Subscriber, stateTopic, dataReaderQos);
        DDS_free(dataReaderQos);

        stateTopic_newMessage(&state_message);

        state_data_t state_data;
        for (int i = 0; i < 10; ++i) {
            bool is_newData = stateTopic_read(state_DataReader, &state_message, &state_data);
            if (is_newData){
                printf("Got some new data. Stamp: %ld Speed: %f\n", state_data.timestamp, state_data.speed);
            }
            sleep(5);
        }

        stateTopic_freeMessage(&state_message);
        deleteDataReader(state_Subscriber, state_DataReader);
        deleteSubscriber(domainParticipant, state_Subscriber);

    }

//cleanup:
    deleteTopic(domainParticipant, stateTopic);
    deleteParticipant(domainParticipant);

    return 0;
}