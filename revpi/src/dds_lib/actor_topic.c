#include "actor_topic.h"
#include "CheckStatus.h"
#include "actor.h"
#include "DDS_entities_manager.h"


topic_t actors_topic_create(const domain_participant_t* domain_participant) {

    DDS_ReturnCode_t status;

    // Register Topic's name in DDS Domain
    DDS_TypeSupport message_type_support = RevPiDDS_ActorsTypeSupport__alloc();
    checkHandle(message_type_support, "RevPiDDS_ActorsTypeSupport__alloc");
    // registerMessageType(message_type_support);
    char* type_name = RevPiDDS_ActorsTypeSupport_get_type_name(message_type_support);

    status = RevPiDDS_ActorsTypeSupport_register_type(message_type_support, domain_participant->dds_domainParticipant, type_name);
    checkStatus(status,"RevPiDDS_ActorsTypeSupport_register_type");

    DDS_free(type_name);

    char* message_type_name = RevPiDDS_ActorsTypeSupport_get_type_name(message_type_support);
    // createTopic();
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

// actor_topic_publisher_t actors_topic_create_publisher(const topic_t* actor_topic) {

//     return topic_publisher_create_new(actor_topic);

// }

// void actors_topic_publish_actor(const actors_topic_t* topic,
//                                 const dds_actor_topic_publisher_t* publisher,
//                                 const ddslib_actor_t* actor) {

//     RevPiDDS_Actors* message = RevPiDDS_Actors__alloc();
//     CheckHandle(message, "RevPiDDS_Actors__alloc");
//     message->address = actor->address;
//     switch (actor->role) {
//         case WORKER:
//             message->role = "worker";
//             break;
//         default:
//             message->role = "none";
//     }

//     DDS_InstanceHandle_t message_handle = RevPiDDS_ActorsDataWriter_register_instance(publisher->actor_DataWriter);

//     DDS_ReturnCode_t status = RevPiDDS_ActorsDataWriter_write(publisher->actor_dataWriter, message, message_handle);
//     checkStatus(status, "RevPiDDS_ActorsDataWriter_write");


//     // Dispose and unregister message
//     status = RevPiDDS_ActorsDataWriter_dispose(publisher->actor_dataWriter, message, message_handle);
//     checkStatus(status, "RevPiDDS_ActorsDataWriter_dispose");
//     status = RevPiDDS_ActorsDataWriter_unregister_instance(publisher->actor_dataWriter, message, message_handle);
//     checkStatus(status, "RevPiDDS_ActorsDataWriter_unregister_instance");

//     DDS_free(message);

// }