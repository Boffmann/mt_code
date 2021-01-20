#include "StateTopic.h"
#include "DDSEntitiesCreator.h"
#include "CheckStatus.h"

// char* g_StateTypeName = DDS_OBJECT_NIL;
// DDS_TypeSupport g_StateTypeSupport = DDS_OBJECT_NIL;

void registerMessageType(DDS_DomainParticipant domainParticipant, DDS_TypeSupport typeSupport) {

    char* typeName = RevPiDDS_StateTypeSupport_get_type_name(typeSupport);

    g_status = RevPiDDS_StateTypeSupport_register_type(typeSupport, domainParticipant, typeName);
    checkStatus(g_status, "RevPiDDS_StateTypeSupport_register_type");

    DDS_free(typeName);

}

DDS_TopicQos* createTopicQos(DDS_DomainParticipant domainParticipant) {

    DDS_TopicQos* topicQos = DDS_TopicQos__alloc();
    checkHandle(topicQos, "DDS_TopicQos__alloc");
    g_status = DDS_DomainParticipant_get_default_topic_qos(domainParticipant, topicQos);
    checkStatus(g_status, "DDS_DomainParticipant_get_default_topic_qos");
    topicQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    topicQos->durability.kind = DDS_TRANSIENT_DURABILITY_QOS;

    // DeadlineQoSPolicy : period used to trigger the listener
    // (on_requested_deadline_missed)
    topicQos->deadline.period.nanosec = 0;
    topicQos->deadline.period.sec = 1;

    return topicQos;

}

DDS_Topic stateTopic_create(DDS_DomainParticipant domainParticipant, const char* topicName) {

    DDS_Topic topic;
    char* stateTypeName = DDS_OBJECT_NIL;
    DDS_TypeSupport stateTypeSupport = DDS_OBJECT_NIL;

    // Register the Topic's type in the DDS Domain.
    stateTypeSupport = RevPiDDS_StateTypeSupport__alloc();
    checkHandle(stateTypeSupport, "RevPiDDS_StateTypeSupport__alloc");
    registerMessageType(domainParticipant, stateTypeSupport);

    DDS_TopicQos* topicQos = createTopicQos(domainParticipant);

    // Create the Topic's in the DDS Domain.
    stateTypeName = RevPiDDS_StateTypeSupport_get_type_name(stateTypeSupport);
    topic = createTopic(domainParticipant, topicName, stateTypeName, topicQos);

    DDS_free(topicQos);
    DDS_free(stateTypeName);
    DDS_free(stateTypeSupport);

    return topic;
}

DDS_PublisherQos* stateTopic_getPublisherQos(DDS_DomainParticipant domainParticipant) {

    DDS_PublisherQos* publisherQos = DDS_PublisherQos__alloc();

    checkHandle(publisherQos, "DDS_PublisherQos__alloc");
    g_status = DDS_DomainParticipant_get_default_publisher_qos(domainParticipant, publisherQos);
    checkStatus(g_status, "DDS_DomainParticipant_get_default_publisher_qos");
    publisherQos->partition.name._length = 1;
    publisherQos->partition.name._maximum = 1;
    publisherQos->partition.name._release = TRUE;
    publisherQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    checkHandle(publisherQos->partition.name._buffer, "DDS_StringSeq_allocbuf");
    publisherQos->partition.name._buffer[0] = DDS_string_dup(g_partitionName);
    checkHandle(publisherQos->partition.name._buffer[0], "DDS_string_dup");

    return publisherQos;
}

DDS_DataWriterQos* stateTopic_getDataWriterQos(DDS_Publisher publisher, DDS_Topic topic) {

    DDS_TopicQos *topicQos = DDS_TopicQos__alloc();
    DDS_DataWriterQos *dataWriterQos = DDS_DataWriterQos__alloc();

    checkHandle(dataWriterQos, "DDS_DataWriterQos__alloc");
    g_status = DDS_Publisher_get_default_datawriter_qos(publisher, dataWriterQos);
    checkStatus(g_status, "DDS_Publisher_get_default_datawriter_qos");
    g_status = DDS_Topic_get_qos(topic, topicQos);
    checkStatus(g_status, "DDS_Topic_get_qos");
    g_status = DDS_Publisher_copy_from_topic_qos(publisher, dataWriterQos, topicQos);
    checkStatus(g_status, "DDS_Publisher_copy_from_topic_qos");
    dataWriterQos->writer_data_lifecycle.autodispose_unregistered_instances = FALSE;

    DDS_free(topicQos);

    return dataWriterQos;
}

void stateTopic_write(DDS_DataWriter dataWriter) {
    RevPiDDS_State* stateMessage;

    stateMessage = RevPiDDS_State__alloc();
    stateMessage->timestamp = 0;
    stateMessage->speed = 12.0;

    g_status = RevPiDDS_StateDataWriter_write(dataWriter, stateMessage, DDS_HANDLE_NIL);
    checkStatus(g_status, "RevPiDDS_StateDataWriter_write");

    DDS_free(stateMessage);
}