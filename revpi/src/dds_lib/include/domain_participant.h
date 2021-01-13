#ifndef __RASPI_DDSLIB_DOMAIN_PARTICIPANT_H__
#define __RASPI_DDSLIB_DOMAIN_PARTICIPANT_H__

#include "dds_dcps.h"

typedef struct {

    // The name of the partition that this participant is part of
    const char* partition_name;
    // The domain participant factory used to create this domain participant
    DDS_DomainParticipantFactory domain_participant_factory;
    // The DDS domain this actor is part of
    DDS_DomainId_t dds_domainId;
    // The DDS Domain participant representing this participant
    DDS_DomainParticipant dds_domainParticipant;

} domain_participant_t;

domain_participant_t domain_participant_create_new(char* partition_name);
void domain_participant_delete(const domain_participant_t* domain_participant);

#endif