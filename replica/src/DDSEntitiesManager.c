#include "DDSEntitiesManager.h"

#include <stdbool.h>

void DDSSetup() {

    createParticipant("Test_Partition");
    createInputTopic();
    // createReplicaResultTopic();

    input_ReadCondition = DDS_DataReader_create_readcondition(
        input_DataReader,
        DDS_NOT_READ_SAMPLE_STATE | DDS_READ_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE | DDS_NOT_NEW_VIEW_STATE, // TODO Validate if this is correct
        DDS_ALIVE_INSTANCE_STATE
        // DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE
        // DDS_ALIVE_INSTANCE_STATE | DDS_NOT_ALIVE_INSTANCE_STATE | DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE
        // DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE
    );
    checkHandle(input_ReadCondition, "DDS_DataReader_create_readcondition (input)");

    input_WaitSet = DDS_WaitSet__alloc();
    checkHandle(input_WaitSet, "DDS_WaitSet__alloc input");

    input_GuardList = DDS_ConditionSeq__alloc();
    checkHandle(input_GuardList, "DDS_ConditionSeq__alloc");
    input_GuardList->_maximum = 1;
    input_GuardList->_length = 0;
    input_GuardList->_release = TRUE;
    input_GuardList->_buffer = DDS_ConditionSeq_allocbuf(1);
    checkHandle(input_GuardList->_buffer, "DDS_ConditionSeq_allocbuf");
}

void DDSCleanup() {

    g_status = RevPiDDS_InputDataReader_delete_readcondition(input_DataReader, input_ReadCondition);
    checkStatus(g_status, "RevPiDDS_AppendEntriesDataReader_delete_readcondition (electionTimer)");

    deleteDataReader(input_Subscriber, input_DataReader);
    deleteDataReader(replicaResult_Subscriber, replicaResult_DataReader);
    deleteSubscriber(domainParticipant, input_Subscriber);
    deleteSubscriber(domainParticipant, replicaResult_Subscriber);
    deleteDataWriter(replicaResult_Publisher, replicaResult_DataWriter);
    deletePublisher(domainParticipant, replicaResult_Publisher);
    deleteTopic(domainParticipant, input_Topic);
    deleteTopic(domainParticipant, replicaResult_Topic);

    DDS_free(input_GuardList);
    DDS_free(input_WaitSet);
}

void createInputTopic() {
    char* typeName = DDS_OBJECT_NIL;
    DDS_TypeSupport typeSupport = DDS_OBJECT_NIL;
    DDS_TopicQos* topicQos = DDS_OBJECT_NIL;
    DDS_SubscriberQos* subscriberQos = DDS_OBJECT_NIL;
    DDS_DataReaderQos* dataReaderQos = DDS_OBJECT_NIL;
    DDS_PublisherQos* publisherQos = DDS_OBJECT_NIL;
    DDS_DataWriterQos* dataWriterQos = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;

    typeSupport = RevPiDDS_InputTypeSupport__alloc();
    checkHandle(typeSupport, "RevPiDDS_InputTypeSupport__alloc");

    typeName = RevPiDDS_InputTypeSupport_get_type_name(typeSupport);

    status = RevPiDDS_InputTypeSupport_register_type(typeSupport, domainParticipant, typeName);
    checkStatus(status, "RevPiDDS_InputTypeSupport_register_type");


    topicQos = DDS_TopicQos__alloc();
    checkHandle(topicQos, "DDS_TopicQos__alloc");
    status = DDS_DomainParticipant_get_default_topic_qos(domainParticipant, topicQos);
    checkStatus(status, "DDS_DomainParticipant_get_default_topic_qos");

    topicQos->destination_order.kind = DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    topicQos->durability.kind = DDS_VOLATILE_DURABILITY_QOS;
    topicQos->history.kind = DDS_KEEP_ALL_HISTORY_QOS;
    topicQos->history.depth = 5;
    topicQos->lifespan.duration = (DDS_Duration_t){1, 0};
    topicQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    topicQos->resource_limits.max_samples = 5;
    topicQos->resource_limits.max_instances = 1;
    topicQos->resource_limits.max_samples_per_instance = 5;

    // Create the Topic's in the DDS Domain.
    typeName = RevPiDDS_InputTypeSupport_get_type_name(typeSupport);
    input_Topic = createTopic(domainParticipant, "RevPi_Input", typeName, topicQos);

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

    input_Subscriber = createSubscriber(domainParticipant, subscriberQos);

    dataReaderQos = DDS_DataReaderQos__alloc();
    checkHandle(dataReaderQos, "DDS_DataReaderQos__alloc");
    status = DDS_Subscriber_get_default_datareader_qos(input_Subscriber, dataReaderQos);
    checkStatus(status, "DDS_Subscriber_get_default_datareader_qos (input_Topic)");
    status = DDS_Subscriber_copy_from_topic_qos(input_Subscriber, dataReaderQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    input_DataReader = createDataReader(input_Subscriber, input_Topic, dataReaderQos);

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

    input_Publisher = createPublisher(domainParticipant, publisherQos);

    dataWriterQos = DDS_DataWriterQos__alloc();
    checkHandle(dataWriterQos, "DDS_DataWriterQos__alloc");
    status = DDS_Publisher_get_default_datawriter_qos(input_Publisher, dataWriterQos);
    checkStatus(status, "DDS_Publisher_get_default_datawriter_qos");
    status = DDS_Publisher_copy_from_topic_qos(input_Publisher, dataWriterQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");
    dataWriterQos->writer_data_lifecycle.autodispose_unregistered_instances = false;

    input_DataWriter = createDataWriter(input_Publisher, input_Topic, dataWriterQos);

    DDS_free(topicQos);
    DDS_free(dataReaderQos);
    DDS_free(typeName);
    DDS_free(typeSupport);
    DDS_free(subscriberQos);

}

void createReplicaResultTopic() {
    char* typeName = DDS_OBJECT_NIL;
    DDS_TypeSupport typeSupport = DDS_OBJECT_NIL;
    DDS_TopicQos* topicQos = DDS_OBJECT_NIL;
    DDS_PublisherQos* publisherQos = DDS_OBJECT_NIL;
    DDS_DataWriterQos* dataWriterQos = DDS_OBJECT_NIL;
    DDS_SubscriberQos* subscriberQos = DDS_OBJECT_NIL;
    DDS_DataReaderQos* dataReaderQos = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;

    typeSupport = RevPiDDS_ReplicaResultTypeSupport__alloc();
    checkHandle(typeSupport, "RevPiDDS_ReplicaResultTypeSupport__alloc");

    typeName = RevPiDDS_ReplicaResultTypeSupport_get_type_name(typeSupport);

    status = RevPiDDS_ReplicaResultTypeSupport_register_type(typeSupport, domainParticipant, typeName);
    checkStatus(status, "RevPiDDS_ReplicaResultTypeSupport_register_type");

    topicQos = DDS_TopicQos__alloc();
    checkHandle(topicQos, "DDS_TopicQos__alloc");
    status = DDS_DomainParticipant_get_default_topic_qos(domainParticipant, topicQos);
    checkStatus(status, "DDS_DomainParticipant_get_default_topic_qos");

    // TODO
    topicQos->destination_order.kind = DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    topicQos->durability.kind = DDS_VOLATILE_DURABILITY_QOS;
    topicQos->history.kind = DDS_KEEP_ALL_HISTORY_QOS;
    topicQos->history.depth = 5;
    topicQos->lifespan.duration = (DDS_Duration_t){1, 0};
    topicQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    topicQos->resource_limits.max_samples = 5;
    topicQos->resource_limits.max_instances = 1;
    topicQos->resource_limits.max_samples_per_instance = 5;

    // Create the Topic's in the DDS Domain.
    typeName = RevPiDDS_ReplicaResultTypeSupport_get_type_name(typeSupport);
    replicaResult_Topic = createTopic(domainParticipant, "RevPi_ReplicaResult", typeName, topicQos);

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

    replicaResult_Publisher = createPublisher(domainParticipant, publisherQos);

    dataWriterQos = DDS_DataWriterQos__alloc();
    checkHandle(dataWriterQos, "DDS_DataWriterQos__alloc");
    status = DDS_Publisher_get_default_datawriter_qos(replicaResult_Publisher, dataWriterQos);
    checkStatus(status, "DDS_Publisher_get_default_datawriter_qos");
    status = DDS_Publisher_copy_from_topic_qos(replicaResult_Publisher, dataWriterQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    replicaResult_DataWriter = createDataWriter(replicaResult_Publisher, replicaResult_Topic, dataWriterQos);

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

    replicaResult_Subscriber = createSubscriber(domainParticipant, subscriberQos);

    dataReaderQos = DDS_DataReaderQos__alloc();
    checkHandle(dataReaderQos, "DDS_DataReaderQos__alloc");
    status = DDS_Subscriber_get_default_datareader_qos(replicaResult_Subscriber, dataReaderQos);
    checkStatus(status, "DDS_Subscriber_get_default_datareader_qos (RequestVote Topic)");
    status = DDS_Subscriber_copy_from_topic_qos(replicaResult_Subscriber, dataReaderQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    replicaResult_DataReader = createDataReader(replicaResult_Subscriber, replicaResult_Topic, dataReaderQos);

    // replicaResult_Listener = createDataReaderListener();

    DDS_free(topicQos);
    DDS_free(typeName);
    DDS_free(dataWriterQos);
    DDS_free(dataReaderQos);
    DDS_free(typeName);
    DDS_free(typeSupport);
    DDS_free(publisherQos);
    DDS_free(subscriberQos);
}