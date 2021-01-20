#include "DDSEntitiesCreator.h"
#include "StateTopic.h"
#include "CheckStatus.h"

// TODO Check if works
// TODO Check if all handels get deleted

int main(int argc, char *argv[]) {

    (void)argc;
    (void)argv;

    DDS_DomainParticipant domainParticipant;
    DDS_Topic stateTopic;
    DDS_PublisherQos* publisherQos;
    DDS_Publisher state_Publisher;
    DDS_DataWriterQos* dataWriterQos;
    DDS_DataWriter state_DataWriter;

    domainParticipant = createParticipant("Test_Partition");
    stateTopic = stateTopic_create(domainParticipant, "State_Topic");

    publisherQos = stateTopic_getPublisherQos(domainParticipant);

    state_Publisher = createPublisher(domainParticipant, publisherQos);

    DDS_free(publisherQos);

    dataWriterQos = stateTopic_getDataWriterQos(state_Publisher, stateTopic);

    state_DataWriter = createDataWriter(state_Publisher, stateTopic, dataWriterQos);

    DDS_free(dataWriterQos);

    for (int i = 0; i < 10; ++i) {
        stateTopic_write(state_DataWriter);
        sleep(5);
    }


    deleteDataWriter(state_Publisher, state_DataWriter);
    deletePublisher(domainParticipant, state_Publisher);
    deleteTopic(domainParticipant, stateTopic);
    deleteParticipant(domainParticipant);

    return 0;
}