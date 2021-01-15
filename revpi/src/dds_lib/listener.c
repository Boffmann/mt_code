#include "listener.h"
#include "CheckStatus.h"


void listener_cleanup(const listener_t* listener, const domain_participant_t* domain_participant) {

    DDS_ReturnCode_t status;
    status = DDS_Subscriber_delete_datareader(listener->dds_subscriber, listener->dds_dataReader);
    checkStatus(status, "DDS_Subscriber_delete_datareader");

    status = DDS_DomainParticipant_delete_subscriber(domain_participant->dds_domainParticipant, listener->dds_subscriber);
    checkStatus(status, "DDS_DomainParticipant_delete_subscriber");

    DDS_free(listener->dds_listener);
    
}

// BEGIN REGION Library Interface Functions

listener_t listener_create_new(const domain_participant_t* domain_participant, const DDS_SubscriberQos* sub_qos) {

    listener_t new_listener;

    new_listener.dds_subscriber = DDS_DomainParticipant_create_subscriber(
        domain_participant->dds_domainParticipant,
        sub_qos,
        NULL,
        DDS_STATUS_MASK_NONE
    );
    checkHandle(new_listener.dds_subscriber, "DDS_DomainParticipant_create_subscriber");

    // DDS_free(subscriber_qos);

    return new_listener;

}

void listener_create_dataReader_new(listener_t* listener, const DDS_DataReaderQos* dr_qos, const topic_t* topic) {

    listener->dds_dataReader = DDS_Subscriber_create_datareader(
        listener->dds_subscriber,
        topic->dds_topic,
        dr_qos,
        NULL,
        DDS_STATUS_MASK_NONE
    );
    checkHandle(listener->dds_dataReader, "DDS_Subscriber_create_datareader");

    // DDS_free(dataReader_qos);

    // Create Listener
    listener->dds_listener = DDS_DataReaderListener__alloc();
    checkHandle(listener->dds_listener, "DDS_DataReaderListener__alloc");

}

DDS_DataReaderQos* dr_qos_copy_from_topic_qos(const listener_t* listener, const topic_t* topic) {

    DDS_DataReaderQos* dataReader_qos = DDS_DataReaderQos__alloc();
    checkHandle(dataReader_qos, "DDS_DataReaderQos__alloc");
    DDS_TopicQos* topic_qos = DDS_TopicQos__alloc();
    checkHandle(topic_qos, "DDS_TopicQos__alloc");

    DDS_ReturnCode_t status = DDS_Topic_get_qos(topic->dds_topic, topic_qos);
    checkStatus(status, "DDS_Topic_get_qos");

    status = DDS_Subscriber_get_default_datareader_qos(listener->dds_subscriber, dataReader_qos);
    checkStatus(status, "DDS_Subscriber_get_default_datareader_qos");

    status = DDS_Subscriber_copy_from_topic_qos(listener->dds_subscriber, dataReader_qos, topic_qos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    DDS_free(topic_qos);

    return dataReader_qos;
}

DDS_SubscriberQos* get_default_subscriber_qos(const domain_participant_t* domain_participant) {

    DDS_SubscriberQos* subscriber_qos = DDS_SubscriberQos__alloc();
    checkHandle(subscriber_qos, "DDS_SubscriberQos__alloc");
    DDS_ReturnCode_t status = DDS_DomainParticipant_get_default_subscriber_qos(domain_participant->dds_domainParticipant, subscriber_qos);
    checkStatus(status, "DDS_DomainParticipant_get_default_subscriber_qos");
    subscriber_qos->partition.name._length = 1;
    subscriber_qos->partition.name._maximum = 1;
    subscriber_qos->partition.name._release = TRUE;
    subscriber_qos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    checkHandle(subscriber_qos->partition.name._buffer, "DDS_StringSeq_allocbuf");
    subscriber_qos->partition.name._buffer[0] = DDS_string_dup(domain_participant->partition_name);
    checkHandle(subscriber_qos->partition.name._buffer[0], "DDS_string_dup");

    return subscriber_qos;
}
// END REGION Library Interface Functions