#include "actor_topic.h"
#include "CheckStatus.h"
#include "actor.h"
#include "datamodel.h"


topic_t actors_topic_create(const domain_participant_t* domain_participant) {

    DDS_ReturnCode_t status;

    // Register Topic's name in DDS Domain
    DDS_TypeSupport message_type_support = RevPiDDS_ActorsTypeSupport__alloc();
    checkHandle(message_type_support, "RevPiDDS_ActorsTypeSupport__alloc");
    char* type_name = RevPiDDS_ActorsTypeSupport_get_type_name(message_type_support);

    status = RevPiDDS_ActorsTypeSupport_register_type(message_type_support, domain_participant->dds_domainParticipant, type_name);
    checkStatus(status,"RevPiDDS_ActorsTypeSupport_register_type");

    DDS_free(type_name);

    char* message_type_name = RevPiDDS_ActorsTypeSupport_get_type_name(message_type_support);
    DDS_TopicQos* topic_qos = DDS_TopicQos__alloc();
    checkHandle(topic_qos, "DDS_TopicQos__alloc");
    status = DDS_DomainParticipant_get_default_topic_qos(domain_participant->dds_domainParticipant, topic_qos);
    checkStatus(status, "DDS_DomainParticipant_get_default_topic_qos");

    // TODO Adjust QoS
    topic_qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    topic_qos->durability.kind = DDS_TRANSIENT_DURABILITY_QOS;

    DDS_free(message_type_support);

    topic_t new_topic = topic_create_new(
        domain_participant->dds_domainParticipant,
        "Actors_Topic",
        message_type_name,
        topic_qos
    );

    DDS_free(message_type_name);
    DDS_free(topic_qos);

    return new_topic;
}