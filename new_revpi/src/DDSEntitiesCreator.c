#include "DDSEntitiesCreator.h"
#include "CheckStatus.h"

DDS_DomainId_t g_domainID = DDS_DOMAIN_ID_DEFAULT;
DDS_DomainParticipantFactory g_domainParticipantFactory = DDS_OBJECT_NIL;

// TODO Check if needs to cleanup
const char* g_partitionName = DDS_OBJECT_NIL;

DDS_ReturnCode_t g_status;

DDS_DomainParticipant createParticipant(const char* partitionName) {

    DDS_DomainParticipant domainParticipant = DDS_OBJECT_NIL;

    g_domainParticipantFactory = DDS_DomainParticipantFactory_get_instance();
    checkHandle(g_domainParticipantFactory, "DDS_DomainParticipantFactory_get_instance");

    domainParticipant = DDS_DomainParticipantFactory_create_participant(
            g_domainParticipantFactory,
            g_domainID,
            DDS_PARTICIPANT_QOS_DEFAULT,
            NULL,
            DDS_STATUS_MASK_NONE
        );
    checkHandle(domainParticipant, "DDS_DomainParticipantFactory_create_participant");

    g_partitionName = partitionName;

    return domainParticipant;

}

void deleteParticipant(DDS_DomainParticipant domainParticipant) {
    g_status = DDS_DomainParticipantFactory_delete_participant(g_domainParticipantFactory, domainParticipant);
    checkStatus(g_status, "DDS_DomainParticipantFactory_delete_participant");
}

DDS_Topic createTopic(DDS_DomainParticipant domainParticipant, const char* topicName, const char* typeName, const DDS_TopicQos* topicQos) {
    DDS_Topic topic;
    const char* messageFirstPart;
    char* message;
    size_t messageFirstPartLength, topicNameLength;

    topic = DDS_DomainParticipant_create_topic(
        domainParticipant,
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

void deleteTopic(DDS_DomainParticipant domainParticipant, DDS_Topic topic) {
    g_status = DDS_DomainParticipant_delete_topic(domainParticipant, topic);
    checkStatus(g_status, "DDS_DomainParticipant_delete_topic");
}

DDS_Publisher createPublisher(DDS_DomainParticipant domainParticipant, const DDS_PublisherQos* publisherQos) {
    DDS_Publisher publisher;
    
    publisher = DDS_DomainParticipant_create_publisher(
        domainParticipant,
        publisherQos,
        NULL,
        DDS_STATUS_MASK_NONE
    );
    checkHandle(publisher, "DDS_DomainParticipant_create_publisher");

    return publisher;
}

void deletePublisher(DDS_DomainParticipant domainParticipant, DDS_Publisher publisher) {
    g_status = DDS_DomainParticipant_delete_publisher(domainParticipant, publisher);
    checkStatus(g_status, "DDS_DomainParticipant_delete_publisher");
}

DDS_DataWriter createDataWriter(DDS_Publisher publisher, DDS_Topic topic, DDS_DataWriterQos* dataWriterQos) {

    DDS_DataWriter dataWriter;

    dataWriter = DDS_Publisher_create_datawriter(
        publisher,
        topic,
        dataWriterQos,
        NULL,
        DDS_STATUS_MASK_NONE
    );
    checkHandle(dataWriter, "DDS_Publisher_create_datawriter");

    return dataWriter;
}

void deleteDataWriter(DDS_Publisher publisher, DDS_DataWriter dataWriter) {
    g_status = DDS_Publisher_delete_datawriter(publisher, dataWriter);
    checkStatus(g_status, "DDS_Publisher_delete_datawriter");
}

DDS_Subscriber createSubscriber(DDS_DomainParticipant domainParticipant, const DDS_SubscriberQos* subscriberQos) {

    DDS_Subscriber subscriber;

    subscriber = DDS_DomainParticipant_create_subscriber(
        domainParticipant,
        subscriberQos,
        NULL,
        DDS_STATUS_MASK_NONE
    );
    checkHandle(subscriber, "DDS_DomainParticipant_create_subscriber");

    return subscriber;
}

void deleteSubscriber(DDS_DomainParticipant domainParticipant, DDS_Subscriber subscriber) {
    g_status = DDS_DomainParticipant_delete_subscriber(domainParticipant, subscriber);
    checkStatus(g_status, "DDS_DomainParticipant_delete_subscriber");
}

DDS_DataReader createDataReader(DDS_Subscriber subscriber, DDS_Topic topic, DDS_DataReaderQos* dataReaderQos) {

    DDS_DataReader dataReader;
    DDS_TopicQos* topicQos;

    topicQos = DDS_TopicQos__alloc();
    checkHandle(topicQos, "DDS_TopicQos__alloc");
    g_status = DDS_Topic_get_qos(topic, topicQos);
    checkStatus(g_status, "DDS_Topic_get_qos");
    g_status = DDS_Subscriber_copy_from_topic_qos(subscriber, dataReaderQos, topicQos);
    checkStatus(g_status, "DDS_Subscriber_copy_from_topic_qos");

    dataReader = DDS_Subscriber_create_datareader(
        subscriber,
        topic,
        dataReaderQos,
        NULL,
        DDS_STATUS_MASK_NONE
    );
    checkHandle(dataReader, "DDS_Subscriber_create_datareader");

    DDS_free(topicQos);

    return dataReader;
}

void deleteDataReader(DDS_Subscriber subscriber, DDS_DataReader dataReader) {
    g_status = DDS_Subscriber_delete_datareader(subscriber, dataReader);
    checkStatus(g_status, "DDS_Subscriber_delete_datareader");
}