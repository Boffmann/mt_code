#include "domain_participant.h"
#include "dds_lib.h"
#include "topic.h"
#include "CheckStatus.h"



domain_participant_t domain_participant_create_new(char* partition_name) {

    domain_participant_t new_participant;
    
    new_participant.domain_participant_factory = DDS_DomainParticipantFactory_get_instance();
    checkHandle(new_participant.domain_participant_factory, "DDS_DomainParticipantFactory_get_instance");

    new_participant.dds_domainId = DDS_DOMAIN_ID_DEFAULT;

    new_participant.dds_domainParticipant = DDS_DomainParticipantFactory_create_participant(
            new_participant.domain_participant_factory,
            new_participant.dds_domainId,
            DDS_PARTICIPANT_QOS_DEFAULT,
            NULL,
            DDS_STATUS_MASK_NONE
        );
    checkHandle(new_participant.dds_domainParticipant, "DDS_DomainParticipantFactory_create_participant");

    new_participant.partition_name = partition_name;

    return new_participant;
}

void domain_participant_delete(const domain_participant_t* domain_participant) {
    DDS_ReturnCode_t status = DDS_DomainParticipantFactory_delete_participant(domain_participant->domain_participant_factory, domain_participant->dds_domainParticipant);
    checkStatus(status, "DDS_DomainParticipantFactory_delete_participant");
}

// BEGIN REGION Library Interface Functions

domain_participant_t setup_dds_domain(char* partition_name) {
    return domain_participant_create_new(partition_name);
} 

// END REGION Library Interface Functions