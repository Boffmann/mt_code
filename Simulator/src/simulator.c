#include "simulator.h"

#include <stdbool.h>

#include "CheckStatus.h"
#include "DDSEntitiesCreator.h"

DDS_DomainParticipant domainParticipant = DDS_OBJECT_NIL;
uint32_t inputID = 0;

DDS_Topic inputTopic;
DDS_Publisher input_Publisher;
DDS_DataWriter input_DataWriter;

const char* partitionName = DDS_OBJECT_NIL;

void simulator_init() {
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
    topicQos->durability.kind = DDS_PERSISTENT_DURABILITY_QOS;
    topicQos->history.kind = DDS_KEEP_ALL_HISTORY_QOS;
    topicQos->history.depth = 5;
    /* topicQos->lifespan.duration = (DDS_Duration_t){1, 0}; */
    topicQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    // topicQos->resource_limits.max_samples = 5;
    // topicQos->resource_limits.max_instances = 1;
    // topicQos->resource_limits.max_samples_per_instance = 5;

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


void send_movement_authority(scenario_t* scenario) {

    // (void)scenario;
    RevPiDDS_Input* input_message = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;
    DDS_InstanceHandle_t test_instance;
    uint8_t payload_size;

    payload_size = 4;
    input_message = RevPiDDS_Input__alloc();
    input_message->id = inputID;
    input_message->data._length = payload_size;
    input_message->data._maximum = payload_size;
    input_message->data._buffer = DDS_sequence_long_allocbuf(payload_size);

    input_message->data._buffer[0] = 1;
    input_message->data._buffer[1] = 2;
    input_message->data._buffer[2] = scenario->movement_authority.start_pos;
    input_message->data._buffer[3] = scenario->movement_authority.end_pos;

    test_instance = RevPiDDS_InputDataWriter_register_instance(input_DataWriter, input_message);

    status = RevPiDDS_InputDataWriter_write(input_DataWriter, input_message, test_instance);
    checkStatus(status, "RevPiDDS_InputDataWriter_write");

    // TODO Autodispose unregistered off
    // status = RevPiDDS_InputDataWriter_unregister_instance(input_DataWriter, input_message, test_instance);
    // checkStatus(status, "RevPiDDS_InputDataWriter_unregister_instance");

    inputID++;

    DDS_free(input_message);
}

void send_linking_information(balise_array_t* linked_balises) {

    RevPiDDS_Input* input_message = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;
    DDS_InstanceHandle_t test_instance;
    uint8_t payload_size;

    payload_size = 2 + linked_balises->used * 2;
    input_message = RevPiDDS_Input__alloc();
    input_message->id = inputID;
    input_message->data._length = payload_size;
    input_message->data._maximum = payload_size;
    input_message->data._buffer = DDS_sequence_long_allocbuf(payload_size);

    input_message->data._buffer[0] = 2;
    input_message->data._buffer[1] = linked_balises->used * 2;

    for (size_t i = 0; i < linked_balises->used; ++i) {
        size_t index = 2 * i + 2;
        input_message->data._buffer[index] = linked_balises->array[i].id;
        input_message->data._buffer[index + 1] = linked_balises->array[i].position;
    }

    test_instance = RevPiDDS_InputDataWriter_register_instance(input_DataWriter, input_message);

    status = RevPiDDS_InputDataWriter_write(input_DataWriter, input_message, test_instance);
    checkStatus(status, "RevPiDDS_InputDataWriter_write");

    inputID++;

    // TODO
    // status = RevPiDDS_InputDataWriter_unregister_instance(input_DataWriter, input_message, test_instance);
    // checkStatus(status, "RevPiDDS_InputDataWriter_unregister_instance");

    DDS_free(input_message);

}

void send_balise(const balise_t* const balise) {

    DDS_InstanceHandle_t test_instance;
    RevPiDDS_Input* input_message = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;
    uint8_t payload_size;

    payload_size = 3;
    input_message = RevPiDDS_Input__alloc();
    input_message->id = inputID;
    input_message->data._length = payload_size;
    input_message->data._maximum = payload_size;
    input_message->data._buffer = DDS_sequence_long_allocbuf(payload_size);

    input_message->data._buffer[0] = 3;
    input_message->data._buffer[1] = 2;
    input_message->data._buffer[2] = balise->id;
    // input_message->data._buffer[3] = balise->position;

    printf("Send a balise with ID %d and position %d\n", balise->id, balise->position);

    test_instance = RevPiDDS_InputDataWriter_register_instance(input_DataWriter, input_message);

    status = RevPiDDS_InputDataWriter_write(input_DataWriter, input_message, test_instance);
    checkStatus(status, "RevPiDDS_InputDataWriter_write");

    inputID++;

    // TODO
    // status = RevPiDDS_InputDataWriter_unregister_instance(input_DataWriter, inputMessage, test_instance);
    // checkStatus(status, "RevPiDDS_InputDataWriter_unregister_instance");

    DDS_free(input_message);
}

void send_terminate() {
    DDS_InstanceHandle_t test_instance;
    RevPiDDS_Input* input_message = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;
    uint8_t payload_size;

    payload_size = 1;
    input_message = RevPiDDS_Input__alloc();
    input_message->id = inputID;
    input_message->data._length = payload_size;
    input_message->data._maximum = payload_size;
    input_message->data._buffer = DDS_sequence_long_allocbuf(payload_size);

    input_message->data._buffer[0] = 0;

    test_instance = RevPiDDS_InputDataWriter_register_instance(input_DataWriter, input_message);

    status = RevPiDDS_InputDataWriter_write(input_DataWriter, input_message, test_instance);
    checkStatus(status, "RevPiDDS_InputDataWriter_write");

    inputID++;

    // TODO
    // status = RevPiDDS_InputDataWriter_unregister_instance(input_DataWriter, inputMessage, test_instance);
    // checkStatus(status, "RevPiDDS_InputDataWriter_unregister_instance");

    DDS_free(input_message);
}