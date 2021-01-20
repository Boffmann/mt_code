#include "DecisionTopic.h"
#include "DDSEntitiesCreator.h"
#include "CheckStatus.h"
#include "ListenerData.h"

on_decision_data_available_t decision_data_available_callback = NULL;

void on_decision_data_available(void* listener_data, DDS_DataReader reader) {
    (void) listener_data;
    struct DecisionMessage message;
    decision_data_t data;
    DDS_sequence_RevPiDDS_Decision  msgSeq          = { 0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq               infoSeq         = { 0, 0, DDS_OBJECT_NIL, FALSE};

    printf("On Data Available\n");


    message.message_seq = &msgSeq;
    message.message_infoSeq = &infoSeq;

    bool is_newData = decisionTopic_read(reader, &message, &data);

    if (is_newData) {
        decision_data_available_callback(&data);
    }

}

void registerDecisionMessageType(DDS_DomainParticipant domainParticipant, DDS_TypeSupport typeSupport) {

    char* typeName = RevPiDDS_DecisionTypeSupport_get_type_name(typeSupport);

    g_status = RevPiDDS_DecisionTypeSupport_register_type(typeSupport, domainParticipant, typeName);
    checkStatus(g_status, "RevPiDDS_DecisionTypeSupport_register_type");

    DDS_free(typeName);

}

DDS_TopicQos* createDecisionTopicQos(DDS_DomainParticipant domainParticipant) {

    DDS_TopicQos* topicQos = DDS_TopicQos__alloc();
    checkHandle(topicQos, "DDS_DecisionQos__alloc");
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

DDS_Topic decisionTopic_create(DDS_DomainParticipant domainParticipant, const char* topicName) {

    DDS_Topic topic;
    char* decisionTypeName = DDS_OBJECT_NIL;
    DDS_TypeSupport decisionTypeSupport = DDS_OBJECT_NIL;

    // Register the Topic's type in the DDS Domain.
    decisionTypeSupport = RevPiDDS_DecisionTypeSupport__alloc();
    checkHandle(decisionTypeSupport, "RevPiDDS_DecisionTypeSupport__alloc");
    registerDecisionMessageType(domainParticipant, decisionTypeSupport);

    DDS_TopicQos* topicQos = createDecisionTopicQos(domainParticipant);

    // Create the Topic's in the DDS Domain.
    decisionTypeName = RevPiDDS_DecisionTypeSupport_get_type_name(decisionTypeSupport);
    topic = createTopic(domainParticipant, topicName, decisionTypeName, topicQos);

    DDS_free(topicQos);
    DDS_free(decisionTypeName);
    DDS_free(decisionTypeSupport);

    return topic;
}

DDS_PublisherQos* decisionTopic_getPublisherQos(DDS_DomainParticipant domainParticipant) {

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

DDS_DataWriterQos* decisionTopic_getDataWriterQos(DDS_Publisher publisher, DDS_Topic topic) {

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

DDS_SubscriberQos* decisionTopic_getSubscriberQos(DDS_DomainParticipant domainParticipant) {

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

DDS_DataReaderQos* decisionTopic_getDataReaderQos(DDS_Subscriber subscriber, DDS_Topic topic) {

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

void decisionTopic_newMessage(struct DecisionMessage* message) {
    message->message_seq = DDS_sequence_RevPiDDS_Decision__alloc();
    checkHandle(message->message_seq, "DDS_sequence_RevPiDDS_Decision__alloc");
    message->message_infoSeq = DDS_SampleInfoSeq__alloc();
    checkHandle(message->message_infoSeq, "DDS_SampleInfoSeq__alloc");
}

void decisionTopic_freeMessage(struct DecisionMessage* message) {
    DDS_free(message->message_seq);
    DDS_free(message->message_infoSeq);
}

void decisionTopic_write(DDS_DataWriter dataWriter, const decision_data_t* data) {
    RevPiDDS_Decision* decisionMessage;

    decisionMessage = RevPiDDS_Decision__alloc();
    decisionMessage->decisionID = data->decisionID;

    g_status = RevPiDDS_DecisionDataWriter_write(dataWriter, decisionMessage, DDS_HANDLE_NIL);
    checkStatus(g_status, "RevPiDDS_DecisionDataWriter_write");

    DDS_free(decisionMessage);
}

bool decisionTopic_read(DDS_DataReader dataReader, struct DecisionMessage* message, decision_data_t* result_data) {

    bool is_newData = false;

    g_status = RevPiDDS_DecisionDataReader_read(
        dataReader,
        message->message_seq,
        message->message_infoSeq,
        1,
        DDS_ANY_SAMPLE_STATE,
        DDS_ANY_VIEW_STATE,
        DDS_ANY_INSTANCE_STATE
    );
    checkStatus(g_status, "RevPiDDS_DecisionDataReader_read");

    if (message->message_seq->_length > 0 && message->message_infoSeq->_buffer[0].valid_data) {
        result_data->decisionID = message->message_seq->_buffer[0].decisionID;
        is_newData = true;
    }
    RevPiDDS_DecisionDataReader_return_loan(dataReader, message->message_seq, message->message_infoSeq);

    return is_newData;
}

void decisionTopic_registerListener(struct DDS_DataReaderListener* listener, DDS_DataReader dataReader, on_decision_data_available_t callback) {

    DDS_StatusMask mask;

    listener->listener_data = NULL;
    listener->on_data_available = on_decision_data_available;

    mask = DDS_DATA_AVAILABLE_STATUS | DDS_REQUESTED_DEADLINE_MISSED_STATUS;
    g_status = DDS_DataReader_set_listener(dataReader, listener, mask);
    checkStatus(g_status, "DDS_DataReader_set_listener");

    decision_data_available_callback = callback;
}