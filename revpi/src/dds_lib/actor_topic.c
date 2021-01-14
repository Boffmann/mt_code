#include "actor_topic.h"
#include "CheckStatus.h"
#include "actor.h"
#include "datamodel.h"


topic_t actors_topic_create(const domain_participant_t* domain_participant, const DDS_TopicQos* topic_qos) {

    DDS_ReturnCode_t status;

    // Register Topic's name in DDS Domain
    DDS_TypeSupport message_type_support = RevPiDDS_ActorsTypeSupport__alloc();
    checkHandle(message_type_support, "RevPiDDS_ActorsTypeSupport__alloc");
    char* type_name = RevPiDDS_ActorsTypeSupport_get_type_name(message_type_support);

    status = RevPiDDS_ActorsTypeSupport_register_type(message_type_support, domain_participant->dds_domainParticipant, type_name);
    checkStatus(status,"RevPiDDS_ActorsTypeSupport_register_type");

    DDS_free(type_name);

    char* message_type_name = RevPiDDS_ActorsTypeSupport_get_type_name(message_type_support);

    DDS_free(message_type_support);

    topic_t new_topic = topic_create_new(
        domain_participant->dds_domainParticipant,
        "Actors_Topic",
        message_type_name,
        topic_qos
    );

    DDS_free(message_type_name);

    return new_topic;
}