#ifndef __RASPI_DDSLIB_LISTENER_H__
#define __RASPI_DDSLIB_LISTENER_H__

#include "dds_dcps.h"
#include "domain_participant.h"
#include "topic.h"

typedef struct {
    DDS_Subscriber dds_subscriber;
    DDS_DataReader dds_dataReader;
    struct DDS_DataReaderListener *dds_listener;
} listener_t;

listener_t listener_create_new(const domain_participant_t* domain_participant, const topic_t* topic);
void listener_cleanup(const listener_t* listener, const domain_participant_t* domain_participant);

#endif