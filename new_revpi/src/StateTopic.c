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

    DDS_TopicQos* topicQos;
    DDS_DataWriterQos* dataWriterQos;

    dataWriterQos = DDS_DataWriterQos__alloc();
    checkHandle(dataWriterQos, "DDS_DataWriterQos__alloc");
    topicQos = DDS_TopicQos__alloc();
    checkHandle(topicQos, "DDS_TopicQos__alloc");
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

DDS_SubscriberQos* stateTopic_getSubscriberQos(DDS_DomainParticipant domainParticipant) {

    DDS_SubscriberQos* subscriberQos = DDS_SubscriberQos__alloc();
    checkHandle(subscriberQos, "DDS_SubscriberQos__alloc");

    g_status = DDS_DomainParticipant_get_default_subscriber_qos(domainParticipant, subscriberQos);
    checkStatus(g_status, "DDS_DomainParticipant_get_default_subscriber_qos");
    subscriberQos->partition.name._length = 1;
    subscriberQos->partition.name._maximum = 1;
    subscriberQos->partition.name._release = TRUE;
    subscriberQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    checkHandle(subscriberQos->partition.name._buffer, "DDS_StringSeq_allocbuf");
    subscriberQos->partition.name._buffer[0] = DDS_string_dup(g_partitionName);
    checkHandle(subscriberQos->partition.name._buffer[0], "DDS_string_dup");

    return subscriberQos;
}

DDS_DataReaderQos* stateTopic_getDataReaderQos(DDS_Subscriber subscriber, DDS_Topic topic) {

    DDS_DataReaderQos* dataReaderQos;
    DDS_TopicQos* topicQos;

    dataReaderQos = DDS_DataReaderQos__alloc();
    checkHandle(dataReaderQos, "DDS_DataReaderQos__alloc");
    topicQos = DDS_TopicQos__alloc();
    checkHandle(topicQos, "DDS_TopicQos__alloc");
    g_status = DDS_Topic_get_qos(topic, topicQos);
    checkStatus(g_status, "DDS_Topic_get_qos");
    g_status = DDS_Subscriber_get_default_datareader_qos(subscriber, dataReaderQos);
    checkStatus(g_status, "DDS_Subscriber_get_default_datareader_qos");
    g_status = DDS_Subscriber_copy_from_topic_qos(subscriber, dataReaderQos, topicQos);
    checkStatus(g_status, "DDS_Publisher_copy_from_topic_qos");

    DDS_free(topicQos);

    return dataReaderQos;
}

void stateTopic_newMessage(struct StateMessage* message) {
    message->message_seq = DDS_sequence_RevPiDDS_State__alloc();
    checkHandle(message->message_seq, "DDS_sequence_RevPiDDS_State__alloc");
    message->message_infoSeq = DDS_SampleInfoSeq__alloc();
    checkHandle(message->message_infoSeq, "DDS_SampleInfoSeq__alloc");
}

void stateTopic_freeMessage(struct StateMessage* message) {
    DDS_free(message->message_seq);
    DDS_free(message->message_infoSeq);
}

void stateTopic_write(DDS_DataWriter dataWriter, const state_data_t* data) {
    RevPiDDS_State* stateMessage;

    stateMessage = RevPiDDS_State__alloc();
    stateMessage->timestamp = data->timestamp;
    stateMessage->speed = data->speed;

    g_status = RevPiDDS_StateDataWriter_write(dataWriter, stateMessage, DDS_HANDLE_NIL);
    checkStatus(g_status, "RevPiDDS_StateDataWriter_write");

    DDS_free(stateMessage);
}

bool stateTopic_read(DDS_DataReader dataReader, struct StateMessage* message, state_data_t* result_data) {

    bool is_newData = false;

    g_status = RevPiDDS_StateDataReader_read(
        dataReader,
        message->message_seq,
        message->message_infoSeq,
        1,
        DDS_ANY_SAMPLE_STATE,
        DDS_ANY_VIEW_STATE,
        DDS_ANY_INSTANCE_STATE
    );
    checkStatus(g_status, "RevPiDDS_StateDataReader_read");

    if (message->message_seq->_length > 0 && message->message_infoSeq->_buffer[0].valid_data) {
        result_data->timestamp = message->message_seq->_buffer[0].timestamp;
        result_data->speed = message->message_seq->_buffer[0].speed;
        RevPiDDS_StateDataReader_return_loan(dataReader, message->message_seq, message->message_infoSeq);
        is_newData = true;
    }

    return is_newData;
}