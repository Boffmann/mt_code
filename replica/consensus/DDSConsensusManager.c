#include "DDSConsensusManager.h"
#include "datamodel.h"

void createAppendEntriesTopic();
void createAppendEntriesReplyTopic();
void createRequestVoteTopic();
void createRequestVoteReplyTopic();
void createReceiveVotesDDSFeatures();

void DDSSetupConsensus() {

    createAppendEntriesTopic();
    createAppendEntriesReplyTopic();
    createRequestVoteTopic();
    createRequestVoteReplyTopic();

}

void DDSConsensusCleanup() {
    g_status = DDS_WaitSet_detach_condition(appendEntries_WaitSet, electionTimer_QueryCondition);
    checkStatus(g_status, "DDS_WaitSet_detach_condition (appendEntries_Waitset/electionTimer_QueryCondition)");
    g_status = DDS_WaitSet_detach_condition(appendEntriesReply_WaitSet, appendEntriesReply_QueryCondition);
    checkStatus(g_status, "DDS_WaitSet_detach_condition (appendEntriesReply_Waitset/ QUeryCondition)");
    g_status = DDS_WaitSet_detach_condition(leaderElection_WaitSet, electionTimer_QueryCondition);
    checkStatus(g_status, "DDS_WaitSet_detach_condition (leaderElection_Waitset/electionTimer_QueryCondition)");
    g_status = DDS_WaitSet_detach_condition(collectVotes_WaitSet, requestVote_QueryCondition);
    checkStatus(g_status, "DDS_WaitSet_detach_condition (collectVotes_Waitset/requestVote_QueryCondition)");
    g_status = DDS_WaitSet_detach_condition(collectVotes_WaitSet, requestVoteReply_QueryCondition);
    checkStatus(g_status, "DDS_WaitSet_detach_condition (collectVotes_WaitSet/requestVoteReply_QueryCondition)");

    g_status = RevPiDDS_AppendEntriesDataReader_delete_readcondition(appendEntries_DataReader, electionTimer_QueryCondition);
    checkStatus(g_status, "RevPiDDS_AppendEntriesDataReader_delete_readcondition (electionTimer)");
    g_status = RevPiDDS_RequestVoteDataReader_delete_readcondition(requestVote_DataReader, requestVote_QueryCondition);
    checkStatus(g_status, "RevPiDDS_AppendEntriesDataReader_delete_readcondition (requestVote)");
    g_status = RevPiDDS_RequestVoteReplyDataReader_delete_readcondition(requestVoteReply_DataReader, requestVoteReply_QueryCondition);
    checkStatus(g_status, "RevPiDDS_AppendEntriesDataReader_delete_readcondition (requestVoteReply)");

    deleteDataReader(appendEntries_Subscriber, appendEntries_DataReader);
    deleteDataReader(requestVote_Subscriber, requestVote_DataReader);
    deleteDataReader(requestVoteReply_Subscriber, requestVoteReply_DataReader);
    deleteSubscriber(domainParticipant, appendEntries_Subscriber);
    deleteSubscriber(domainParticipant, requestVote_Subscriber);
    deleteSubscriber(domainParticipant, requestVoteReply_Subscriber);
    deleteDataWriter(appendEntries_Publisher, appendEntries_DataWriter);
    deleteDataWriter(requestVote_Publisher, requestVote_DataWriter);
    deleteDataWriter(requestVoteReply_Publisher, requestVoteReply_DataWriter);
    deletePublisher(domainParticipant, appendEntries_Publisher);
    deletePublisher(domainParticipant, requestVote_Publisher);
    deletePublisher(domainParticipant, requestVoteReply_Publisher);
    deleteTopic(domainParticipant, appendEntries_Topic);
    deleteTopic(domainParticipant, requestVote_Topic);
    deleteTopic(domainParticipant, requestVoteReply_Topic);

    DDS_free(appendEntries_GuardList);
    DDS_free(leaderElection_GuardList);
    DDS_free(collectVotes_GuardList);

    DDS_free(appendEntries_WaitSet);
    DDS_free(leaderElection_WaitSet);
    DDS_free(collectVotes_WaitSet);

}

void createAppendEntriesTopic() {
    char* typeName = DDS_OBJECT_NIL;
    DDS_TypeSupport typeSupport = DDS_OBJECT_NIL;
    DDS_TopicQos* topicQos = DDS_OBJECT_NIL;
    DDS_SubscriberQos* subscriberQos = DDS_OBJECT_NIL;
    DDS_DataReaderQos* dataReaderQos = DDS_OBJECT_NIL;
    DDS_PublisherQos* publisherQos = DDS_OBJECT_NIL;
    DDS_DataWriterQos* dataWriterQos = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;

    typeSupport = RevPiDDS_AppendEntriesTypeSupport__alloc();
    checkHandle(typeSupport, "RevPiDDS_AppendEntriesTypeSupport__alloc");

    typeName = RevPiDDS_AppendEntriesTypeSupport_get_type_name(typeSupport);

    status = RevPiDDS_AppendEntriesTypeSupport_register_type(typeSupport, domainParticipant, typeName);
    checkStatus(status, "RevPiDDS_AppendEntriesTypeSupport_register_type");

    topicQos = DDS_TopicQos__alloc();
    checkHandle(topicQos, "DDS_TopicQos__alloc");
    status = DDS_DomainParticipant_get_default_topic_qos(domainParticipant, topicQos);
    checkStatus(status, "DDS_DomainParticipant_get_default_topic_qos");

    // TODO
    topicQos->deadline.period.sec = 3;
    topicQos->deadline.period.nanosec = 0;
    topicQos->destination_order.kind = DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    topicQos->durability.kind = DDS_VOLATILE_DURABILITY_QOS;
    topicQos->history.kind = DDS_KEEP_ALL_HISTORY_QOS;
    topicQos->history.depth = 5;
    topicQos->lifespan.duration = (DDS_Duration_t){1, 0};
    topicQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    topicQos->resource_limits.max_samples = 15;
    topicQos->resource_limits.max_instances = 1;
    topicQos->resource_limits.max_samples_per_instance = 15;

    // Create the Topic's in the DDS Domain.
    typeName = RevPiDDS_AppendEntriesTypeSupport_get_type_name(typeSupport);
    appendEntries_Topic = createTopic(domainParticipant, "RevPi_AppendEntries", typeName, topicQos);

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

    appendEntries_Subscriber = createSubscriber(domainParticipant, subscriberQos);

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

    appendEntries_Publisher = createPublisher(domainParticipant, publisherQos);

    dataWriterQos = DDS_DataWriterQos__alloc();
    checkHandle(dataWriterQos, "DDS_DataWriterQos__alloc");
    status = DDS_Publisher_get_default_datawriter_qos(appendEntries_Publisher, dataWriterQos);
    checkStatus(status, "DDS_Publisher_get_default_datawriter_qos");
    status = DDS_Publisher_copy_from_topic_qos(appendEntries_Publisher, dataWriterQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    appendEntries_DataWriter = createDataWriter(appendEntries_Publisher, appendEntries_Topic, dataWriterQos);

    dataReaderQos = DDS_DataReaderQos__alloc();
    checkHandle(dataReaderQos, "DDS_DataReaderQos__alloc");
    status = DDS_Subscriber_get_default_datareader_qos(appendEntries_Subscriber, dataReaderQos);
    checkStatus(status, "DDS_Subscriber_get_default_datareader_qos (AppendEntries_Topic)");
    status = DDS_Subscriber_copy_from_topic_qos(appendEntries_Subscriber, dataReaderQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    appendEntries_DataReader = createDataReader(appendEntries_Subscriber, appendEntries_Topic, dataReaderQos);

    DDS_free(topicQos);
    DDS_free(dataReaderQos);
    DDS_free(dataWriterQos);
    DDS_free(typeName);
    DDS_free(typeSupport);
    DDS_free(subscriberQos);
    DDS_free(publisherQos);

}

void createAppendEntriesReplyTopic() {
    char* typeName = DDS_OBJECT_NIL;
    DDS_TypeSupport typeSupport = DDS_OBJECT_NIL;
    DDS_TopicQos* topicQos = DDS_OBJECT_NIL;
    DDS_SubscriberQos* subscriberQos = DDS_OBJECT_NIL;
    DDS_DataReaderQos* dataReaderQos = DDS_OBJECT_NIL;
    DDS_PublisherQos* publisherQos = DDS_OBJECT_NIL;
    DDS_DataWriterQos* dataWriterQos = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;

    typeSupport = RevPiDDS_AppendEntriesReplyTypeSupport__alloc();
    checkHandle(typeSupport, "RevPiDDS_AppendEntriesReplyTypeSupport__alloc");

    typeName = RevPiDDS_AppendEntriesReplyTypeSupport_get_type_name(typeSupport);

    status = RevPiDDS_AppendEntriesReplyTypeSupport_register_type(typeSupport, domainParticipant, typeName);
    checkStatus(status, "RevPiDDS_AppendEntriesReplyTypeSupport_register_type");

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
    topicQos->resource_limits.max_samples = 15;
    topicQos->resource_limits.max_instances = 1;
    topicQos->resource_limits.max_samples_per_instance = 15;

    // Create the Topic's in the DDS Domain.
    typeName = RevPiDDS_AppendEntriesReplyTypeSupport_get_type_name(typeSupport);
    appendEntriesReply_Topic = createTopic(domainParticipant, "RevPi_AppendEntriesReply", typeName, topicQos);

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

    appendEntriesReply_Subscriber = createSubscriber(domainParticipant, subscriberQos);

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

    appendEntriesReply_Publisher = createPublisher(domainParticipant, publisherQos);

    dataWriterQos = DDS_DataWriterQos__alloc();
    checkHandle(dataWriterQos, "DDS_DataWriterQos__alloc");
    status = DDS_Publisher_get_default_datawriter_qos(appendEntriesReply_Publisher, dataWriterQos);
    checkStatus(status, "DDS_Publisher_get_default_datawriter_qos");
    status = DDS_Publisher_copy_from_topic_qos(appendEntriesReply_Publisher, dataWriterQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    appendEntries_DataWriter = createDataWriter(appendEntriesReply_Publisher, appendEntriesReply_Topic, dataWriterQos);

    dataReaderQos = DDS_DataReaderQos__alloc();
    checkHandle(dataReaderQos, "DDS_DataReaderQos__alloc");
    status = DDS_Subscriber_get_default_datareader_qos(appendEntriesReply_Subscriber, dataReaderQos);
    checkStatus(status, "DDS_Subscriber_get_default_datareader_qos (AppendEntriesReply_Topic)");
    status = DDS_Subscriber_copy_from_topic_qos(appendEntriesReply_Subscriber, dataReaderQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    appendEntriesReply_DataReader = createDataReader(appendEntriesReply_Subscriber, appendEntriesReply_Topic, dataReaderQos);

    DDS_free(topicQos);
    DDS_free(dataReaderQos);
    DDS_free(dataWriterQos);
    DDS_free(typeName);
    DDS_free(typeSupport);
    DDS_free(subscriberQos);
    DDS_free(publisherQos);

}

void createRequestVoteTopic() {
    char* typeName = DDS_OBJECT_NIL;
    DDS_TypeSupport typeSupport = DDS_OBJECT_NIL;
    DDS_TopicQos* topicQos = DDS_OBJECT_NIL;
    DDS_PublisherQos* publisherQos = DDS_OBJECT_NIL;
    DDS_DataWriterQos* dataWriterQos = DDS_OBJECT_NIL;
    DDS_SubscriberQos* subscriberQos = DDS_OBJECT_NIL;
    DDS_DataReaderQos* dataReaderQos = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;

    typeSupport = RevPiDDS_RequestVoteTypeSupport__alloc();
    checkHandle(typeSupport, "RevPiDDS_RequestVoteTypeSupport__alloc");

    typeName = RevPiDDS_RequestVoteTypeSupport_get_type_name(typeSupport);

    status = RevPiDDS_RequestVoteTypeSupport_register_type(typeSupport, domainParticipant, typeName);
    checkStatus(status, "RevPiDDS_RequestVoteTypeSupport_register_type");


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
    topicQos->resource_limits.max_samples = 15;
    topicQos->resource_limits.max_instances = 1;
    topicQos->resource_limits.max_samples_per_instance = 15;

    // Create the Topic's in the DDS Domain.
    typeName = RevPiDDS_RequestVoteTypeSupport_get_type_name(typeSupport);
    requestVote_Topic = createTopic(domainParticipant, "RevPi_RequestVote", typeName, topicQos);

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

    requestVote_Publisher = createPublisher(domainParticipant, publisherQos);

    dataWriterQos = DDS_DataWriterQos__alloc();
    checkHandle(dataWriterQos, "DDS_DataWriterQos__alloc");
    status = DDS_Publisher_get_default_datawriter_qos(requestVote_Publisher, dataWriterQos);
    checkStatus(status, "DDS_Publisher_get_default_datawriter_qos");
    status = DDS_Publisher_copy_from_topic_qos(requestVote_Publisher, dataWriterQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    requestVote_DataWriter = createDataWriter(requestVote_Publisher, requestVote_Topic, dataWriterQos);

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

    requestVote_Subscriber = createSubscriber(domainParticipant, subscriberQos);

    dataReaderQos = DDS_DataReaderQos__alloc();
    checkHandle(dataReaderQos, "DDS_DataReaderQos__alloc");
    status = DDS_Subscriber_get_default_datareader_qos(requestVote_Subscriber, dataReaderQos);
    checkStatus(status, "DDS_Subscriber_get_default_datareader_qos (RequestVote Topic)");
    status = DDS_Subscriber_copy_from_topic_qos(requestVote_Subscriber, dataReaderQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    requestVote_DataReader = createDataReader(requestVote_Subscriber, requestVote_Topic, dataReaderQos);

    // requestVote_Listener = createDataReaderListener();

    DDS_free(topicQos);
    DDS_free(typeName);
    DDS_free(dataWriterQos);
    DDS_free(dataReaderQos);
    DDS_free(typeSupport);
    DDS_free(publisherQos);
    DDS_free(subscriberQos);

}

void createRequestVoteReplyTopic() {
    char* typeName = DDS_OBJECT_NIL;
    DDS_TypeSupport typeSupport = DDS_OBJECT_NIL;
    DDS_TopicQos* topicQos = DDS_OBJECT_NIL;
    DDS_PublisherQos* publisherQos = DDS_OBJECT_NIL;
    DDS_DataWriterQos* dataWriterQos = DDS_OBJECT_NIL;
    DDS_SubscriberQos* subscriberQos = DDS_OBJECT_NIL;
    DDS_DataReaderQos* dataReaderQos = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;

    typeSupport = RevPiDDS_RequestVoteReplyTypeSupport__alloc();
    checkHandle(typeSupport, "RevPiDDS_RequestVoteReplyTypeSupport__alloc");

    typeName = RevPiDDS_RequestVoteReplyTypeSupport_get_type_name(typeSupport);

    status = RevPiDDS_RequestVoteReplyTypeSupport_register_type(typeSupport, domainParticipant, typeName);
    checkStatus(status, "RevPiDDS_RequestVoteReplyTypeSupport_register_type");

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
    topicQos->resource_limits.max_samples = 15;
    topicQos->resource_limits.max_instances = 1;
    topicQos->resource_limits.max_samples_per_instance = 15;

    // Create the Topic's in the DDS Domain.
    typeName = RevPiDDS_RequestVoteReplyTypeSupport_get_type_name(typeSupport);
    requestVoteReply_Topic = createTopic(domainParticipant, "RevPi_RequestVoteReply", typeName, topicQos);

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

    requestVoteReply_Publisher = createPublisher(domainParticipant, publisherQos);

    dataWriterQos = DDS_DataWriterQos__alloc();
    checkHandle(dataWriterQos, "DDS_DataWriterQos__alloc");
    status = DDS_Publisher_get_default_datawriter_qos(requestVote_Publisher, dataWriterQos);
    checkStatus(status, "DDS_Publisher_get_default_datawriter_qos");
    status = DDS_Publisher_copy_from_topic_qos(requestVote_Publisher, dataWriterQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    requestVoteReply_DataWriter = createDataWriter(requestVoteReply_Publisher, requestVoteReply_Topic, dataWriterQos);

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

    requestVoteReply_Subscriber = createSubscriber(domainParticipant, subscriberQos);

    dataReaderQos = DDS_DataReaderQos__alloc();
    checkHandle(dataReaderQos, "DDS_DataReaderQos__alloc");
    status = DDS_Subscriber_get_default_datareader_qos(requestVoteReply_Subscriber, dataReaderQos);
    checkStatus(status, "DDS_Subscriber_get_default_datareader_qos (RequestVote Topic)");
    status = DDS_Subscriber_copy_from_topic_qos(requestVoteReply_Subscriber, dataReaderQos, topicQos);
    checkStatus(status, "DDS_Publisher_copy_from_topic_qos");

    requestVoteReply_DataReader = createDataReader(requestVoteReply_Subscriber, requestVoteReply_Topic, dataReaderQos);

    DDS_free(topicQos);
    DDS_free(dataWriterQos);
    DDS_free(dataReaderQos);
    DDS_free(typeName);
    DDS_free(typeSupport);
    DDS_free(publisherQos);
    DDS_free(subscriberQos);

}

void createLeaderElectionDDSFeatures(const uint8_t ID) {

    char ID_string[2];
    sprintf(ID_string, "%d", ID);
    DDS_char* ID_queryParameter = ID_string;
    DDS_char* ID_queryString = "senderID!=%0";

    DDS_StringSeq* query_parameters = DDS_StringSeq__alloc();

    query_parameters->_length = 1;
    query_parameters->_maximum = 1;
    query_parameters->_release = TRUE;
    query_parameters->_buffer = DDS_StringSeq_allocbuf (1);
    checkHandle(query_parameters->_buffer, "DDS_StringSeq_allocbuf");
    query_parameters->_buffer[0] = DDS_string_dup (ID_queryParameter);
    checkHandle(query_parameters->_buffer[0], "DDS_string_dup");

    leaderElection_WaitSet = DDS_WaitSet__alloc();
    checkHandle(leaderElection_WaitSet, "DDS_WaitSet__alloc collect Votes");

    appendEntries_WaitSet = DDS_WaitSet__alloc();
    checkHandle(appendEntries_WaitSet, "DDS_WaitSet__alloc appendEntries");

    appendEntriesReply_WaitSet = DDS_WaitSet__alloc();
    checkHandle(appendEntriesReply_WaitSet, "DDS_WaitSet__alloc appendEntriesReply");

    collectVotes_WaitSet = DDS_WaitSet__alloc();
    checkHandle(collectVotes_WaitSet, "DDS_WaitSet__alloc requestVote");

    collectVotes_GuardList = DDS_ConditionSeq__alloc();
    checkHandle(collectVotes_GuardList, "DDS_ConditionSeq__alloc");
    collectVotes_GuardList->_maximum = 2;
    collectVotes_GuardList->_length = 0;
    collectVotes_GuardList->_release = TRUE;
    collectVotes_GuardList->_buffer = DDS_ConditionSeq_allocbuf(2);
    checkHandle(collectVotes_GuardList->_buffer, "DDS_ConditionSeq_allocbuf");

    leaderElection_GuardList = DDS_ConditionSeq__alloc();
    checkHandle(leaderElection_GuardList, "DDS_ConditionSeq__alloc");
    leaderElection_GuardList->_maximum = 1;
    leaderElection_GuardList->_length = 0;
    leaderElection_GuardList->_release = TRUE;
    leaderElection_GuardList->_buffer = DDS_ConditionSeq_allocbuf(1);
    checkHandle(leaderElection_GuardList->_buffer, "DDS_ConditionSeq_allocbuf");

    appendEntries_GuardList = DDS_ConditionSeq__alloc();
    checkHandle(appendEntries_GuardList, "DDS_ConditionSeq__alloc");
    appendEntries_GuardList->_maximum = 1;
    appendEntries_GuardList->_length = 0;
    appendEntries_GuardList->_release = TRUE;
    appendEntries_GuardList->_buffer = DDS_ConditionSeq_allocbuf(1);
    checkHandle(appendEntries_GuardList->_buffer, "DDS_ConditionSeq_allocbuf");

    appendEntriesReply_GuardList = DDS_ConditionSeq__alloc();
    checkHandle(appendEntriesReply_GuardList, "DDS_ConditionSeq__alloc");
    appendEntriesReply_GuardList->_maximum = 1;
    appendEntriesReply_GuardList->_length = 0;
    appendEntriesReply_GuardList->_release = TRUE;
    appendEntriesReply_GuardList->_buffer = DDS_ConditionSeq_allocbuf(1);
    checkHandle(appendEntriesReply_GuardList->_buffer, "DDS_ConditionSeq_allocbuf");

    requestVoteReply_QueryCondition = DDS_DataReader_create_querycondition(
        requestVoteReply_DataReader,
        DDS_NOT_READ_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE | DDS_NOT_NEW_VIEW_STATE, // TODO Validate if this is correct
        DDS_ALIVE_INSTANCE_STATE,
        ID_queryString,
        query_parameters

    );
    checkHandle(requestVoteReply_QueryCondition, "DDS_DataReader_create_querycondition (requestVoteReply)");

    requestVote_QueryCondition = DDS_DataReader_create_querycondition(
        requestVote_DataReader,
        DDS_NOT_READ_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE | DDS_NOT_NEW_VIEW_STATE, // TODO Validate if this is correct
        DDS_ALIVE_INSTANCE_STATE,
        ID_queryString,
        query_parameters
    );
    checkHandle(requestVote_QueryCondition, "DDS_DataReader_create_querycondition (requestVote)");

    electionTimer_QueryCondition = DDS_DataReader_create_querycondition(
        appendEntries_DataReader,
        DDS_NOT_READ_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE | DDS_NOT_NEW_VIEW_STATE,
        DDS_ALIVE_INSTANCE_STATE,
        ID_queryString,
        query_parameters
    );
    checkHandle(electionTimer_QueryCondition, "DDS_DataReader_create_querycondition (electionTimer)");

    appendEntriesReply_QueryCondition = DDS_DataReader_create_querycondition(
        appendEntriesReply_DataReader,
        DDS_NOT_READ_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE | DDS_NOT_NEW_VIEW_STATE,
        DDS_ALIVE_INSTANCE_STATE,
        ID_queryString,
        query_parameters
    );
    checkHandle(appendEntriesReply_QueryCondition, "DDS_DataReader_create_querycondition (appendEntriesReply)");

    g_status = DDS_WaitSet_attach_condition(collectVotes_WaitSet, requestVote_QueryCondition);
    checkStatus(g_status, "DDS_WaitSet_attach_condition (requestVote_QueryCondition)");
    g_status = DDS_WaitSet_attach_condition(collectVotes_WaitSet, requestVoteReply_QueryCondition);
    checkStatus(g_status, "DDS_WaitSet_attach_condition (RequestVoteReply QueryCondition)");

    g_status = DDS_WaitSet_attach_condition(leaderElection_WaitSet, electionTimer_QueryCondition);
    checkStatus(g_status, "DDS_WaitSet_attach_condition (append Entries readCondition)");

    g_status = DDS_WaitSet_attach_condition(appendEntries_WaitSet, electionTimer_QueryCondition);
    checkStatus(g_status, "DDS_WaitSet_attach_condition (appendEntries_QueryCondition)");

    g_status = DDS_WaitSet_attach_condition(appendEntriesReply_WaitSet, appendEntriesReply_QueryCondition);
    checkStatus(g_status, "DDS_WaitSet_attach_condition (appendEntriesReply_QueryCondition)");

    DDS_free(query_parameters);

}