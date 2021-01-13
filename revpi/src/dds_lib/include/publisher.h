#ifndef __RASPI_DDSLIB_PUBLISHER_H__
#define __RASPI_DDSLIB_PUBLISHER_H__

#include <stdbool.h>

#include "dds_dcps.h"
#include "domain_participant.h"
#include "topic.h"


typedef struct {
    DDS_Publisher dds_publisher;
    DDS_DataWriter dds_dataWriter;
    bool has_dataWriter;
} publisher_t;

publisher_t publisher_create_new(const domain_participant_t* domain_participant);
void publisher_cleanup(publisher_t* publisher, const domain_participant_t* domain_participant);

void publisher_add_datawriter(publisher_t* publisher, const topic_t* topic);


#endif