#include "listener.h"
#include "CheckStatus.h"


listener_t listener_create_new(const domain_participant_t* domain_participant, const topic_t* topic) {

    listener_t new_listener;
    DDS_ReturnCode_t status;

    DDS_SubscriberQos* subscriber_qos = DDS_SubscriberQos__alloc();
    checkHandle(subscriber_qos, "DDS_SubscriberQos__alloc");
    status = DDS_DomainParticipant_get_default_subscriber_qos(domain_participant->dds_domainParticipant, subscriber_qos);
    checkStatus(status, "DDS_DomainParticipant_get_default_subscriber_qos");
    subscriber_qos->partition.name._length = 1;
    subscriber_qos->partition.name._maximum = 1;
    subscriber_qos->partition.name._release = TRUE;
    subscriber_qos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    checkHandle(subscriber_qos->partition.name._buffer, "DDS_StringSeq_allocbuf");
    subscriber_qos->partition.name._buffer[0] = DDS_string_dup(domain_participant->partition_name);
    checkHandle(subscriber_qos->partition.name._buffer[0], "DDS_string_dup");
    new_listener.dds_subscriber = DDS_DomainParticipant_create_subscriber(
        domain_participant->dds_domainParticipant,
        subscriber_qos,
        NULL,
        DDS_STATUS_MASK_NONE
    );
    checkHandle(new_listener.dds_subscriber, "DDS_DomainParticipant_create_subscriber");

    DDS_free(subscriber_qos);

    // Create DataReader
    DDS_DataReaderQos* dataReader_qos = DDS_DataReaderQos__alloc();
    checkHandle(dataReader_qos, "DDS_DataReaderQos__alloc");
    DDS_TopicQos* topic_qos = DDS_TopicQos__alloc();
    checkHandle(topic_qos, "DDS_TopicQos__alloc");

    status = DDS_Topic_get_qos(topic->dds_topic, topic_qos);
    checkStatus(status, "DDS_Topic_get_qos");

    status = DDS_Subscriber_get_default_datareader_qos(new_listener.dds_subscriber, dataReader_qos);
    checkStatus(status, "DDS_Subscriber_get_default_datareader_qos");

    status = DDS_Subscriber_copy_from_topic_qos(new_listener.dds_subscriber, dataReader_qos, topic_qos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    new_listener.dds_dataReader = DDS_Subscriber_create_datareader(
        new_listener.dds_subscriber,
        topic->dds_topic,
        dataReader_qos,
        NULL,
        DDS_STATUS_MASK_NONE
    );
    checkHandle(new_listener.dds_dataReader, "DDS_Subscriber_create_datareader");

    DDS_free(dataReader_qos);
    DDS_free(topic_qos);

    // Create Listener
    new_listener.dds_listener = DDS_DataReaderListener__alloc();
    checkHandle(new_listener.dds_listener, "DDS_DataReaderListener__alloc");

    DDS_StatusMask mask = DDS_DATA_AVAILABLE_STATUS | DDS_REQUESTED_DEADLINE_MISSED_STATUS;
    status = DDS_DataReader_set_listener(new_listener.dds_dataReader, new_listener.dds_listener, mask);
    checkStatus(status, "DDS_DataReader_set_listener");

    return new_listener;

}

void listener_cleanup(const listener_t* listener, const domain_participant_t* domain_participant) {

    DDS_ReturnCode_t status;
    status = DDS_Subscriber_delete_datareader(listener->dds_subscriber, listener->dds_dataReader);
    checkStatus(status, "DDS_Subscriber_delete_datareader");

    status = DDS_DomainParticipant_delete_subscriber(domain_participant->dds_domainParticipant, listener->dds_subscriber);
    checkStatus(status, "DDS_DomainParticipant_delete_subscriber");

    DDS_free(listener->dds_listener);
    
}

// BEGIN REGION Library Interface Functions

listener_t add_listener(const domain_participant_t* domain_participant, const topic_t* topic) {
    return listener_create_new(domain_participant, topic);
}

// END REGION Library Interface Functions