#include "DDSEntitiesManager.h"
#include "CheckStatus.h"

DDS_DomainId_t g_domainID = DDS_DOMAIN_ID_DEFAULT;
DDS_DomainParticipantFactory g_domainParticipantFactory = DDS_OBJECT_NIL;
DDS_DomainParticipant g_domainParticipant = DDS_OBJECT_NIL;

const char* g_partitionName = DDS_OBJECT_NIL;

DDS_ReturnCode_t g_status;

void createParticipant(const char* partitionName) {
    g_domainParticipantFactory = DDS_DomainParticipantFactory_get_instance();
    checkHandle(g_domainParticipantFactory, "DDS_DomainParticipantFactory_get_instance");

    g_domainParticipant = DDS_DomainParticipantFactory_create_participant(
            g_domainParticipantFactory,
            g_domainID,
            DDS_PARTICIPANT_QOS_DEFAULT,
            NULL,
            DDS_STATUS_MASK_NONE
        );
    checkHandle(g_domainParticipant, "DDS_DomainParticipantFactory_create_participant");

    g_partitionName = partitionName;

}

void deleteParticipant() {
    g_status = DDS_DomainParticipantFactory_delete_participant(g_domainParticipantFactory, g_domainParticipant);
    checkStatus(g_status, "DDS_DomainParticipantFactory_delete_participant");
}

DDS_Topic createTopic(const char* topicName, const char* typeName, const DDS_TopicQos* topicQos) {
    DDS_Topic topic;
    const char* messageFirstPart;
    char* message;
    size_t messageFirstPartLength, topicNameLength;

    topic = DDS_DomainParticipant_create_topic(
        g_domainParticipant,
        topicName,
        typeName,
        topicQos,
        NULL,
        DDS_STATUS_MASK_NONE
    );

    messageFirstPart = "DDS_DomainParticipant_create_topic";
    messageFirstPartLength = strlen(messageFirstPart);
    topicNameLength = strlen(topicName);
    message = (char*) malloc(messageFirstPartLength + topicNameLength + 2);

    snprintf(message, messageFirstPartLength + topicNameLength + 1, "%s %s", messageFirstPart, topicName);
    checkHandle(topic, message);

    free(message);


    //DDS_free(TopicQos);
    
    return topic;
}

void deleteTopic(DDS_Topic topic) {
    g_status = DDS_DomainParticipant_delete_topic(g_domainParticipant, topic);
    checkStatus(g_status, "DDS_DomainParticipant_delete_topic");
}

DDS_Publisher createPublisher(const DDS_PublisherQos* publisherQos) {
    DDS_Publisher publisher;
    
    publisher = DDS_DomainParticipant_create_publisher(
        g_domainParticipant,
        publisherQos,
        NULL,
        DDS_STATUS_MASK_NONE
    );
    checkHandle(publisher, "DDS_DomainParticipant_create_publisher");

    // DDS_free(publisherQos);
    return publisher;
}

void deletePublisher(DDS_Publisher publisher) {
    g_status = DDS_DomainParticipant_delete_publisher(g_domainParticipant, publisher);
    checkStatus(g_status, "DDS_DomainParticipant_delete_publisher");
}

DDS_DataWriter createDataWriter(DDS_Publisher publisher, DDS_Topic topic, DDS_DataWriterQos* dataWriterQos) {
    DDS_DataWriter dataWriter;
    DDS_TopicQos* topicQos = DDS_TopicQos__alloc();    

    g_status = DDS_Topic_get_qos(topic, topicQos);
    checkStatus(g_status, "DDS_Topic_get_qos");
    g_status = DDS_Publisher_copy_from_topic_qos(publisher, dataWriterQos, topicQos);
    checkStatus(g_status, "DDS_Publisher_copy_from_topic_qos");
    dataWriterQos->writer_data_lifecycle.autodispose_unregistered_instances = FALSE;

    dataWriter = DDS_Publisher_create_datawriter(
        publisher,
        topic,
        dataWriterQos,
        NULL,
        DDS_STATUS_MASK_NONE
    );
    checkHandle(dataWriter, "DDS_Publisher_create_datawriter");

    DDS_free(dataWriterQos);
    DDS_free(topicQos);

    return dataWriter;
}

void deleteDataWriter(DDS_Publisher publisher, DDS_DataWriter dataWriter) {
    g_status = DDS_Publisher_delete_datawriter(publisher, dataWriter);
    checkStatus(g_status, "DDS_Publisher_delete_datawriter");
}