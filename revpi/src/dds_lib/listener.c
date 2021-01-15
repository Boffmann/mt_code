#include "listener.h"
#include "CheckStatus.h"


listener_t listener_create_new(const domain_participant_t* domain_participant, const DDS_SubscriberQos* sub_qos) {

    listener_t new_listener;
    subscriber_t new_listener_subscriber = subscriber_create_new(domain_participant, sub_qos);

    new_listener.subscriber = new_listener_subscriber;

    return new_listener;

}

void listener_create_dataReader_new(listener_t* listener, const DDS_DataReaderQos* dr_qos, const topic_t* topic) {

    subscriber_dataReader_create_new(&listener->subscriber, dr_qos, topic);

    // Create Listener
    listener->dds_listener = DDS_DataReaderListener__alloc();
    checkHandle(listener->dds_listener, "DDS_DataReaderListener__alloc");

}

void listener_cleanup(listener_t* listener, const domain_participant_t* domain_participant) {

    subscriber_cleanup(&listener->subscriber, domain_participant);

    DDS_free(listener->dds_listener);
    
}