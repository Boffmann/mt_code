#ifndef __RASPI_DDSLIB_SUBSCRIBER_H__
#define __RASPI_DDSLIB_SUBSCRIBER_H__

#include <stdbool.h>

#include "dds_dcps.h"
#include "domain_participant.h"
#include "topic.h"

typedef struct {
    DDS_Subscriber dds_subscriber;
    DDS_DataReader dds_dataReader;
    bool has_dataReader;
} subscriber_t;

subscriber_t subscriber_create_new(const domain_participant_t* domain_participant, const DDS_SubscriberQos* sub_qos);

void subscriber_dataReader_create_new(subscriber_t* subscriber, const DDS_DataReaderQos* dr_qos, const topic_t* topic);

DDS_SubscriberQos* get_default_subscriber_qos(const domain_participant_t* domain_participant);

void subscriber_cleanup(subscriber_t* subscriber, const domain_participant_t* domain_participant);

#endif