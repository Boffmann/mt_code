#include "topic.h"
#include "decision_topic.h"
#include "task_topic.h"
#include "state_topic.h"
#include "CheckStatus.h"

// Attribute structure

typedef struct {

    // Publisher used to write to the actors topic
    DDS_Publisher topic_publisher;
    // DataWriter used to write to the actors topic
    DDS_DataWriter actor_dataWriter;

} topic_publisher_t;

// BEGIN REGION Library Interface Functions

topic_t topic_join(domain_participant_t* domain_participant, const DDS_TopicQos* topic_qos, const TopicType type) {

    switch (type) {
    case DECISIONS:
        return decision_topic_create(domain_participant, topic_qos);
        break;
    case STATE:
        return state_topic_create(domain_participant, topic_qos);
        break;
    case TASKS:
        return tasks_topic_create(domain_participant, topic_qos);
    default:
        printf("ERROR: The specified topic type does not exist\n");
        break;
    }
    return tasks_topic_create(domain_participant, topic_qos);
}


// END REGION Library Interface Functions

topic_t topic_create_new(domain_participant_t* domain_participant, const char* topicName, const char* typeName, const DDS_TopicQos* topic_qos) {

    topic_t new_topic;

    new_topic.dds_topic = DDS_DomainParticipant_create_topic(
        domain_participant,
        topicName,
        typeName,
        topic_qos,
        NULL,
        DDS_STATUS_MASK_NONE
    );
    checkHandle(new_topic.dds_topic, "DDS::DomainParticipant::create_topic ()");

    //Format error message
    // TODO Prevent malloc
    const char* messageFirstPart = "DDS_DomainParticipant_create_topic";
    size_t messageFirstPartLength = strlen(messageFirstPart);
    size_t topicNameLength = strlen(topicName);
    char* message = (char*) malloc(messageFirstPartLength + topicNameLength + 2);

    snprintf(message, messageFirstPartLength + topicNameLength + 1, "%s %s", messageFirstPart, topicName);
    checkHandle(new_topic.dds_topic, message);

    free(message);

    new_topic.qos_handle = topic_qos;

    return new_topic;

}

void topic_leave(topic_t* topic, domain_participant_t* domain_participant) {
    DDS_ReturnCode_t status = DDS_DomainParticipant_delete_topic(domain_participant->dds_domainParticipant, topic->dds_topic);
    checkStatus(status, "DDS_DomainParticipant_delete_topic");
}