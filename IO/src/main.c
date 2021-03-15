#include <stdbool.h>

#include "dds_dcps.h"
#include "CheckStatus.h"
#include "DDSEntitiesCreator.h"

DDS_DomainParticipant domainParticipant = DDS_OBJECT_NIL;

DDS_Topic inputTopic;
DDS_Publisher input_Publisher;
DDS_DataWriter input_DataWriter;

const char* partitionName = DDS_OBJECT_NIL;

void createInputTopic() {
    char* typeName = DDS_OBJECT_NIL;
    DDS_TypeSupport typeSupport = DDS_OBJECT_NIL;
    DDS_TopicQos* topicQos = DDS_OBJECT_NIL;
    DDS_PublisherQos* publisherQos = DDS_OBJECT_NIL;
    DDS_DataWriterQos* dataWriterQos = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;

    typeSupport = RevPiDDS_InputTypeSupport__alloc();
    checkHandle(typeSupport, "RevPiDDS_InputTypeSupport__alloc");

    typeName = RevPiDDS_InputTypeSupport_get_type_name(typeSupport);

    status = RevPiDDS_InputTypeSupport_register_type(typeSupport, domainParticipant, typeName);
    checkStatus(status, "RevPiDDS_InputTypeSupport_register_type");


    topicQos = DDS_TopicQos__alloc();
    checkHandle(topicQos, "DDS_TopicQos__alloc");
    status = DDS_DomainParticipant_get_default_topic_qos(domainParticipant, topicQos);
    checkStatus(status, "DDS_DomainParticipant_get_default_topic_qos");

    topicQos->destination_order.kind = DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    topicQos->durability.kind = DDS_VOLATILE_DURABILITY_QOS;
    topicQos->history.kind = DDS_KEEP_ALL_HISTORY_QOS;
    topicQos->history.depth = 5;
    /* topicQos->lifespan.duration = (DDS_Duration_t){1, 0}; */
    topicQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    topicQos->resource_limits.max_samples = 5;
    topicQos->resource_limits.max_instances = 1;
    topicQos->resource_limits.max_samples_per_instance = 5;

    // Create the Topic's in the DDS Domain.
    typeName = RevPiDDS_InputTypeSupport_get_type_name(typeSupport);
    inputTopic = createTopic(domainParticipant, "RevPi_Input", typeName, topicQos);
    printf("Created topic\n");

    publisherQos = DDS_PublisherQos__alloc();
    checkHandle(publisherQos, "DDS_PublisherQos__alloc");

    status = DDS_DomainParticipant_get_default_publisher_qos(domainParticipant, publisherQos);
    checkStatus(status, "DDS_DomainParticipant_get_default_publisher_qos");
    publisherQos->partition.name._length = 1;
    publisherQos->partition.name._maximum = 1;
    publisherQos->partition.name._release = TRUE;
    publisherQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    checkHandle(publisherQos->partition.name._buffer, "DDS_StringSeq_allocbuf");
    publisherQos->partition.name._buffer[0] = DDS_string_dup(g_partitionName);
    checkHandle(publisherQos->partition.name._buffer[0], "DDS_string_dup");

    input_Publisher = createPublisher(domainParticipant, publisherQos);

    dataWriterQos = DDS_DataWriterQos__alloc();
    checkHandle(dataWriterQos, "DDS_DataWriterQos__alloc");
    status = DDS_Publisher_get_default_datawriter_qos(input_Publisher, dataWriterQos);
    checkStatus(status, "DDS_Publisher_get_default_datawriter_qos");
    status = DDS_Publisher_copy_from_topic_qos(input_Publisher, dataWriterQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");
    dataWriterQos->writer_data_lifecycle.autodispose_unregistered_instances = false;

    input_DataWriter = createDataWriter(input_Publisher, inputTopic, dataWriterQos);

    DDS_free(topicQos);
    DDS_free(typeName);
    DDS_free(dataWriterQos);
    DDS_free(typeName);
    DDS_free(typeSupport);
    DDS_free(publisherQos);

}

void setupDDS() {

    domainParticipant = createParticipant("Test_Partition");

    createInputTopic();

}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    DDS_InstanceHandle_t test_instance;
    RevPiDDS_Input* inputMessage = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;

    setupDDS();


    test_instance = RevPiDDS_InputDataWriter_register_instance(input_DataWriter, inputMessage);

    inputMessage = RevPiDDS_Input__alloc();
    inputMessage->test = 42;
    status = RevPiDDS_InputDataWriter_write(input_DataWriter, inputMessage, test_instance);
    checkStatus(status, "RevPiDDS_InputDataWriter_write");

    sleep(5);

    status = RevPiDDS_InputDataWriter_unregister_instance(input_DataWriter, inputMessage, test_instance);
    checkStatus(status, "RevPiDDS_InputDataWriter_unregister_instance");

    DDS_free(inputMessage);


}
