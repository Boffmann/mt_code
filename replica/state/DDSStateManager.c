#include "DDSStateManager.h"
#include "datamodel.h"

#include <stdbool.h>

void createLinkedBaliseTopic();
// void createBaliseGroupTopic();
void createMovementAuthorityTopic();
void createTrainStateTopic();

void DDSSetupState() {
    createLinkedBaliseTopic();
    // createBaliseGroupTopic();
    createMovementAuthorityTopic();
    createTrainStateTopic();
}

void DDSStateCleanup() {

    deleteDataReader(linkedBalises_Subscriber, linkedBalises_DataReader);
    // deleteDataReader(baliseGroup_Subscriber, baliseGroup_DataReader);
    deleteDataReader(movementAuthority_Subscriber, movementAuthority_DataReader);
    deleteDataReader(trainState_Subscriber, trainState_DataReader);
    deleteSubscriber(domainParticipant, linkedBalises_Subscriber);
    // deleteSubscriber(domainParticipant, baliseGroup_Subscriber);
    deleteSubscriber(domainParticipant, movementAuthority_Subscriber);
    deleteSubscriber(domainParticipant, trainState_Subscriber);
    deleteDataWriter(linkedBalises_Publisher, linkedBalises_DataWriter);
    // deleteDataWriter(baliseGroup_Publisher, baliseGroup_DataWriter);
    deleteDataWriter(movementAuthority_Publisher, movementAuthority_DataWriter);
    deleteDataWriter(trainState_Publisher, trainState_DataWriter);
    deletePublisher(domainParticipant, linkedBalises_Publisher);
    // deletePublisher(domainParticipant, baliseGroup_Publisher);
    deletePublisher(domainParticipant, movementAuthority_Publisher);
    deletePublisher(domainParticipant, trainState_Publisher);
    deleteTopic(domainParticipant, linkedBalises_Topic);
    // deleteTopic(domainParticipant, baliseGroup_Topic);
    deleteTopic(domainParticipant, movementAuthority_Topic);
    deleteTopic(domainParticipant, trainState_Topic);

}

void createLinkedBaliseTopic() {

    char* typeName = DDS_OBJECT_NIL;
    DDS_TypeSupport typeSupport = DDS_OBJECT_NIL;
    DDS_TopicQos* topicQos = DDS_OBJECT_NIL;
    DDS_SubscriberQos* subscriberQos = DDS_OBJECT_NIL;
    DDS_DataReaderQos* dataReaderQos = DDS_OBJECT_NIL;
    DDS_PublisherQos* publisherQos = DDS_OBJECT_NIL;
    DDS_DataWriterQos* dataWriterQos = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;

    typeSupport = RevPiDDS_LinkedBalisesTypeSupport__alloc();
    checkHandle(typeSupport, "RevPiDDS_LinkedBaliseTypeSupport__alloc");

    typeName = RevPiDDS_LinkedBalisesTypeSupport_get_type_name(typeSupport);

    status = RevPiDDS_LinkedBalisesTypeSupport_register_type(typeSupport, domainParticipant, typeName);
    checkStatus(status, "RevPiDDS_LinkedBalisesTypeSupport_register_type");

    topicQos = DDS_TopicQos__alloc();
    checkHandle(topicQos, "DDS_TopicQos__alloc");
    status = DDS_DomainParticipant_get_default_topic_qos(domainParticipant, topicQos);
    checkStatus(status, "DDS_DomainParticipant_get_default_topic_qos");

    // TODO
    topicQos->destination_order.kind = DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    topicQos->durability.kind = DDS_PERSISTENT_DURABILITY_QOS;
    topicQos->history.kind = DDS_KEEP_LAST_HISTORY_QOS;
    topicQos->history.depth = 1;
    topicQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    // topicQos->resource_limits.max_samples = 5;
    // topicQos->resource_limits.max_instances = 1;
    // topicQos->resource_limits.max_samples_per_instance = 5;

    // Create the Topic's in the DDS Domain.
    typeName = RevPiDDS_LinkedBalisesTypeSupport_get_type_name(typeSupport);
    linkedBalises_Topic = createTopic(domainParticipant, "RevPi_LinkedBalises", typeName, topicQos);

    subscriberQos = DDS_SubscriberQos__alloc();
    checkHandle(subscriberQos, "DDS_SubscriberQos__alloc");

    status = DDS_DomainParticipant_get_default_subscriber_qos(domainParticipant, subscriberQos);
    checkStatus(status, "DDS_DomainParticipant_get_default_subscriber_qos");
    subscriberQos->partition.name._length = 1;
    subscriberQos->partition.name._maximum = 1;
    subscriberQos->partition.name._release = TRUE;
    subscriberQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    checkHandle(subscriberQos->partition.name._buffer, "DDS_StringSeq_allocbuf");
    subscriberQos->partition.name._buffer[0] = DDS_string_dup(g_partitionName);
    checkHandle(subscriberQos->partition.name._buffer[0], "DDS_string_dup");

    linkedBalises_Subscriber = createSubscriber(domainParticipant, subscriberQos);

    publisherQos = DDS_PublisherQos__alloc();
    checkHandle(publisherQos, "DDS_PublisherQos__alloc");

    status = DDS_DomainParticipant_get_default_publisher_qos(domainParticipant, publisherQos);
    checkStatus(status, "DDS_DomainParticipant_get_default_publisher_qos");
    publisherQos->partition.name._length = 1;
    publisherQos->partition.name._maximum = 1;
    publisherQos->partition.name._release = TRUE;
    publisherQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    checkHandle(publisherQos->partition.name._buffer, "DDS_StringSeq_allocbuf");
    publisherQos->partition.name._buffer[0] = DDS_string_dup(g_partitionName);
    checkHandle(publisherQos->partition.name._buffer[0], "DDS_string_dup");

    linkedBalises_Publisher = createPublisher(domainParticipant, publisherQos);

    dataWriterQos = DDS_DataWriterQos__alloc();
    checkHandle(dataWriterQos, "DDS_DataWriterQos__alloc");
    status = DDS_Publisher_get_default_datawriter_qos(linkedBalises_Publisher, dataWriterQos);
    checkStatus(status, "DDS_Publisher_get_default_datawriter_qos");
    status = DDS_Publisher_copy_from_topic_qos(linkedBalises_Publisher, dataWriterQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");
    dataWriterQos->writer_data_lifecycle.autodispose_unregistered_instances = false;

    linkedBalises_DataWriter = createDataWriter(linkedBalises_Publisher, linkedBalises_Topic, dataWriterQos);

    dataReaderQos = DDS_DataReaderQos__alloc();
    checkHandle(dataReaderQos, "DDS_DataReaderQos__alloc");
    status = DDS_Subscriber_get_default_datareader_qos(linkedBalises_Subscriber, dataReaderQos);
    checkStatus(status, "DDS_Subscriber_get_default_datareader_qos (LinkedBalises_Topic)");
    status = DDS_Subscriber_copy_from_topic_qos(linkedBalises_Subscriber, dataReaderQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    linkedBalises_DataReader = createDataReader(linkedBalises_Subscriber, linkedBalises_Topic, dataReaderQos);

    DDS_free(topicQos);
    DDS_free(dataReaderQos);
    DDS_free(dataWriterQos);
    DDS_free(typeName);
    DDS_free(typeSupport);
    DDS_free(subscriberQos);
    DDS_free(publisherQos);

}

void createMovementAuthorityTopic() {
    char* typeName = DDS_OBJECT_NIL;
    DDS_TypeSupport typeSupport = DDS_OBJECT_NIL;
    DDS_TopicQos* topicQos = DDS_OBJECT_NIL;
    DDS_SubscriberQos* subscriberQos = DDS_OBJECT_NIL;
    DDS_DataReaderQos* dataReaderQos = DDS_OBJECT_NIL;
    DDS_PublisherQos* publisherQos = DDS_OBJECT_NIL;
    DDS_DataWriterQos* dataWriterQos = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;

    typeSupport = RevPiDDS_MovementAuthorityTypeSupport__alloc();
    checkHandle(typeSupport, "RevPiDDS_MovementAuthorityTypeSupport__alloc");

    typeName = RevPiDDS_MovementAuthorityTypeSupport_get_type_name(typeSupport);

    status = RevPiDDS_MovementAuthorityTypeSupport_register_type(typeSupport, domainParticipant, typeName);
    checkStatus(status, "RevPiDDS_MovementAuthorityTypeSupport_register_type");

    topicQos = DDS_TopicQos__alloc();
    checkHandle(topicQos, "DDS_TopicQos__alloc");
    status = DDS_DomainParticipant_get_default_topic_qos(domainParticipant, topicQos);
    checkStatus(status, "DDS_DomainParticipant_get_default_topic_qos");

    // TODO
    topicQos->destination_order.kind = DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    topicQos->durability.kind = DDS_VOLATILE_DURABILITY_QOS;
    topicQos->history.kind = DDS_KEEP_LAST_HISTORY_QOS;
    topicQos->history.depth = 1;
    topicQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    // topicQos->resource_limits.max_samples = 5;
    // topicQos->resource_limits.max_instances = 1;
    // topicQos->resource_limits.max_samples_per_instance = 5;

    // Create the Topic's in the DDS Domain.
    typeName = RevPiDDS_MovementAuthorityTypeSupport_get_type_name(typeSupport);
    movementAuthority_Topic = createTopic(domainParticipant, "RevPi_MovementAuthority", typeName, topicQos);

    subscriberQos = DDS_SubscriberQos__alloc();
    checkHandle(subscriberQos, "DDS_SubscriberQos__alloc");

    status = DDS_DomainParticipant_get_default_subscriber_qos(domainParticipant, subscriberQos);
    checkStatus(status, "DDS_DomainParticipant_get_default_subscriber_qos");
    subscriberQos->partition.name._length = 1;
    subscriberQos->partition.name._maximum = 1;
    subscriberQos->partition.name._release = TRUE;
    subscriberQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    checkHandle(subscriberQos->partition.name._buffer, "DDS_StringSeq_allocbuf");
    subscriberQos->partition.name._buffer[0] = DDS_string_dup(g_partitionName);
    checkHandle(subscriberQos->partition.name._buffer[0], "DDS_string_dup");

    movementAuthority_Subscriber = createSubscriber(domainParticipant, subscriberQos);

    publisherQos = DDS_PublisherQos__alloc();
    checkHandle(publisherQos, "DDS_PublisherQos__alloc");

    status = DDS_DomainParticipant_get_default_publisher_qos(domainParticipant, publisherQos);
    checkStatus(status, "DDS_DomainParticipant_get_default_publisher_qos");
    publisherQos->partition.name._length = 1;
    publisherQos->partition.name._maximum = 1;
    publisherQos->partition.name._release = TRUE;
    publisherQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    checkHandle(publisherQos->partition.name._buffer, "DDS_StringSeq_allocbuf");
    publisherQos->partition.name._buffer[0] = DDS_string_dup(g_partitionName);
    checkHandle(publisherQos->partition.name._buffer[0], "DDS_string_dup");

    movementAuthority_Publisher = createPublisher(domainParticipant, publisherQos);

    dataWriterQos = DDS_DataWriterQos__alloc();
    checkHandle(dataWriterQos, "DDS_DataWriterQos__alloc");
    status = DDS_Publisher_get_default_datawriter_qos(movementAuthority_Publisher, dataWriterQos);
    checkStatus(status, "DDS_Publisher_get_default_datawriter_qos");
    status = DDS_Publisher_copy_from_topic_qos(movementAuthority_Publisher, dataWriterQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");
    dataWriterQos->writer_data_lifecycle.autodispose_unregistered_instances = false;

    movementAuthority_DataWriter = createDataWriter(movementAuthority_Publisher, movementAuthority_Topic, dataWriterQos);

    dataReaderQos = DDS_DataReaderQos__alloc();
    checkHandle(dataReaderQos, "DDS_DataReaderQos__alloc");
    status = DDS_Subscriber_get_default_datareader_qos(movementAuthority_Subscriber, dataReaderQos);
    checkStatus(status, "DDS_Subscriber_get_default_datareader_qos (MovementAuthority_Topic)");
    status = DDS_Subscriber_copy_from_topic_qos(movementAuthority_Subscriber, dataReaderQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    movementAuthority_DataReader = createDataReader(movementAuthority_Subscriber, movementAuthority_Topic, dataReaderQos);

    DDS_free(topicQos);
    DDS_free(dataReaderQos);
    DDS_free(dataWriterQos);
    DDS_free(typeName);
    DDS_free(typeSupport);
    DDS_free(subscriberQos);
    DDS_free(publisherQos);

}

void createTrainStateTopic() {
    char* typeName = DDS_OBJECT_NIL;
    DDS_TypeSupport typeSupport = DDS_OBJECT_NIL;
    DDS_TopicQos* topicQos = DDS_OBJECT_NIL;
    DDS_SubscriberQos* subscriberQos = DDS_OBJECT_NIL;
    DDS_DataReaderQos* dataReaderQos = DDS_OBJECT_NIL;
    DDS_PublisherQos* publisherQos = DDS_OBJECT_NIL;
    DDS_DataWriterQos* dataWriterQos = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;

    typeSupport = RevPiDDS_TrainStateTypeSupport__alloc();
    checkHandle(typeSupport, "RevPiDDS_TrainStateTypeSupport__alloc");

    typeName = RevPiDDS_TrainStateTypeSupport_get_type_name(typeSupport);

    status = RevPiDDS_TrainStateTypeSupport_register_type(typeSupport, domainParticipant, typeName);
    checkStatus(status, "RevPiDDS_TrainStateTypeSupport_register_type");

    topicQos = DDS_TopicQos__alloc();
    checkHandle(topicQos, "DDS_TopicQos__alloc");
    status = DDS_DomainParticipant_get_default_topic_qos(domainParticipant, topicQos);
    checkStatus(status, "DDS_DomainParticipant_get_default_topic_qos");

    // TODO
    topicQos->destination_order.kind = DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    topicQos->durability.kind = DDS_VOLATILE_DURABILITY_QOS;
    topicQos->history.kind = DDS_KEEP_LAST_HISTORY_QOS;
    topicQos->history.depth = 1;
    topicQos->reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
    // topicQos->resource_limits.max_samples = 5;
    // topicQos->resource_limits.max_instances = 1;
    // topicQos->resource_limits.max_samples_per_instance = 5;

    // Create the Topic's in the DDS Domain.
    typeName = RevPiDDS_TrainStateTypeSupport_get_type_name(typeSupport);
    trainState_Topic = createTopic(domainParticipant, "RevPi_TrainState", typeName, topicQos);

    subscriberQos = DDS_SubscriberQos__alloc();
    checkHandle(subscriberQos, "DDS_SubscriberQos__alloc");

    status = DDS_DomainParticipant_get_default_subscriber_qos(domainParticipant, subscriberQos);
    checkStatus(status, "DDS_DomainParticipant_get_default_subscriber_qos");
    subscriberQos->partition.name._length = 1;
    subscriberQos->partition.name._maximum = 1;
    subscriberQos->partition.name._release = TRUE;
    subscriberQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    checkHandle(subscriberQos->partition.name._buffer, "DDS_StringSeq_allocbuf");
    subscriberQos->partition.name._buffer[0] = DDS_string_dup(g_partitionName);
    checkHandle(subscriberQos->partition.name._buffer[0], "DDS_string_dup");

    trainState_Subscriber = createSubscriber(domainParticipant, subscriberQos);

    publisherQos = DDS_PublisherQos__alloc();
    checkHandle(publisherQos, "DDS_PublisherQos__alloc");

    status = DDS_DomainParticipant_get_default_publisher_qos(domainParticipant, publisherQos);
    checkStatus(status, "DDS_DomainParticipant_get_default_publisher_qos");
    publisherQos->partition.name._length = 1;
    publisherQos->partition.name._maximum = 1;
    publisherQos->partition.name._release = TRUE;
    publisherQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
    checkHandle(publisherQos->partition.name._buffer, "DDS_StringSeq_allocbuf");
    publisherQos->partition.name._buffer[0] = DDS_string_dup(g_partitionName);
    checkHandle(publisherQos->partition.name._buffer[0], "DDS_string_dup");

    trainState_Publisher = createPublisher(domainParticipant, publisherQos);

    dataWriterQos = DDS_DataWriterQos__alloc();
    checkHandle(dataWriterQos, "DDS_DataWriterQos__alloc");
    status = DDS_Publisher_get_default_datawriter_qos(trainState_Publisher, dataWriterQos);
    checkStatus(status, "DDS_Publisher_get_default_datawriter_qos");
    status = DDS_Publisher_copy_from_topic_qos(trainState_Publisher, dataWriterQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    trainState_DataWriter = createDataWriter(trainState_Publisher, trainState_Topic, dataWriterQos);

    dataReaderQos = DDS_DataReaderQos__alloc();
    checkHandle(dataReaderQos, "DDS_DataReaderQos__alloc");
    status = DDS_Subscriber_get_default_datareader_qos(trainState_Subscriber, dataReaderQos);
    checkStatus(status, "DDS_Subscriber_get_default_datareader_qos (TrainState_Topic)");
    status = DDS_Subscriber_copy_from_topic_qos(trainState_Subscriber, dataReaderQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    trainState_DataReader = createDataReader(trainState_Subscriber, trainState_Topic, dataReaderQos);

    DDS_free(topicQos);
    DDS_free(dataReaderQos);
    DDS_free(dataWriterQos);
    DDS_free(typeName);
    DDS_free(typeSupport);
    DDS_free(subscriberQos);
    DDS_free(publisherQos);

}