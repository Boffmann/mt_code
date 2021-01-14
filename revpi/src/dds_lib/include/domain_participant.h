#ifndef __RASPI_DDSLIB_DOMAIN_PARTICIPANT_H__
#define __RASPI_DDSLIB_DOMAIN_PARTICIPANT_H__

#include "dds_dcps.h"

/**
 * @brief Represents a domain participant in a DDS domain.
 * 
 * This structure combines all OpenSplice DDS features required to
 * represent a domain participant in a DDS domain.
 */
typedef struct {

    const char* partition_name;                                 ///< The name of the partition that this participant is part of
    DDS_DomainParticipantFactory domain_participant_factory;    ///< The domain participant factory used to create this domain participant
    DDS_DomainId_t dds_domainId;                                ///< The DDS domain this actor is part of
    DDS_DomainParticipant dds_domainParticipant;                ///< The DDS Domain participant representing this participant

} domain_participant_t;

/**
 * @brief Creates a new participant and joins a partition in the default DDS domain.
 * 
 * @param partition_name The name of the partition to join.
 * The constructed domain participant can be used to refer to the specified partition for
 * further uses, e.g. managing topics within this domain.
 * 
 * @return Returns a new domain participant in the default DDS domain
 */
domain_participant_t domain_participant_create_new(char* partition_name);

/**
 * @brief Leaves DDS domain and frees resources kept by the domain participant.
 * 
 * @param domain_participant Handle to the domain participant to delete
 */
void domain_participant_delete(const domain_participant_t* domain_participant);

#endif