#include "DDSEntitiesManager.h"
#include "StateTopic.h"
#include "CheckStatus.h"

int main(int argc, char *argv[]) {

    (void)argc;
    (void)argv;

    DDS_Publisher state_Publisher;
    DDS_DataWriter state_DataWriter;

    createParticipant("Test_Partition");
    stateTopic_create("State_Topic");

    DDS_PublisherQos* publisherQos = DDS_PublisherQos__alloc();
    checkHandle(publisherQos, "DDS_PublisherQos__alloc");
    g_status = DDS_DomainParticipant_get_default_publisher_qos(g_domainParticipant, publisherQos);
    checkStatus(g_status, "DDS_DomainParticipant_get_default_publisher_qos");
    publisherQos->partition.name._length = 1;
    publisherQos->partition.name._maximum = 1;
    publisherQos->partition.name._release = TRUE;
    publisherQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    checkHandle(publisherQos->partition.name._buffer, "DDS_StringSeq_allocbuf");
    publisherQos->partition.name._buffer[0] = DDS_string_dup(g_partitionName);
    checkHandle(publisherQos->partition.name._buffer[0], "DDS_string_dup");

    state_Publisher = createPublisher(publisherQos);

    DDS_free(publisherQos);


    DDS_TopicQos *topicQos = DDS_TopicQos__alloc();
    DDS_DataWriterQos *dataWriterQos = DDS_DataWriterQos__alloc();
    checkHandle(dataWriterQos, "DDS_DataWriterQos__alloc");
    g_status = DDS_Publisher_get_default_datawriter_qos(state_Publisher, dataWriterQos);
    checkStatus(g_status, "DDS_Publisher_get_default_datawriter_qos");
    g_status = DDS_Topic_get_qos(g_StateTopic, topicQos);
    checkStatus(g_status, "DDS_Topic_get_qos");
    g_status = DDS_Publisher_copy_from_topic_qos(state_Publisher, dataWriterQos, topicQos);
    checkStatus(g_status, "DDS_Publisher_copy_from_topic_qos");
    dataWriterQos->writer_data_lifecycle.autodispose_unregistered_instances = FALSE;

    state_DataWriter = createDataWriter(state_Publisher, g_StateTopic, dataWriterQos);

    DDS_free(dataWriterQos);
    DDS_free(topicQos);

    for (int i = 0; i < 10; ++i) {
        stateTopic_write(state_DataWriter);
        sleep(5);
    }


    deleteDataWriter(state_Publisher, state_DataWriter);
    deletePublisher(state_Publisher);
    deleteTopic(g_StateTopic);
    deleteParticipant();

    return 0;
}