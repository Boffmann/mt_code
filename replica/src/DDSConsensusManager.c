#include "DDSConsensusManager.h"
#include "datamodel.h"

void DDSSetupConsensus() {

    createAppendEntriesTopic();
    createRequestVoteTopic();
    createRequestVoteReplyTopic();

    start_election_event = DDS_GuardCondition__alloc();
    checkHandle(start_election_event, "DDS_GuardCondition__alloc");

    election_WaitSet = DDS_WaitSet__alloc();
    checkHandle(election_WaitSet, "DDS_WaitSet__alloc election");

    election_GuardList = DDS_ConditionSeq__alloc();
    checkHandle(election_GuardList, "DDS_ConditionSeq__alloc");
    election_GuardList->_maximum = 1;
    election_GuardList->_length = 0;
    election_GuardList->_release = TRUE;
    election_GuardList->_buffer = DDS_ConditionSeq_allocbuf(1);
    checkHandle(election_GuardList->_buffer, "DDS_ConditionSeq_allocbuf");

    g_status = DDS_WaitSet_attach_condition(election_WaitSet, start_election_event);
    checkStatus(g_status, "DDS_WaitSet_attach_condition (start election)");
}

void DDSConsensusCleanup() {
    g_status = DDS_WaitSet_detach_condition(appendEntries_WaitSet, electionTimer_ReadCondition);
    checkStatus(g_status, "DDS_WaitSet_attach_condition (appendEntries_ReadCondition)");
    g_status = DDS_WaitSet_detach_condition(election_WaitSet, start_election_event);
    checkStatus(g_status, "DDS_WaitSet_attach_condition (start election)");

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
    DDS_free(appendEntries_WaitSet);

    g_status = DDS_WaitSet_detach_condition(appendEntries_WaitSet, electionTimer_ReadCondition);
    checkStatus(g_status, "DDS_WaitSet_detach_condition appendEntries_ReadCondition");

    g_status = RevPiDDS_AppendEntriesDataReader_delete_readcondition(appendEntries_DataReader, electionTimer_ReadCondition);
    checkStatus(g_status, "RevPiDDS_AppendEntriesDataReader_delete_readcondition");
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
    topicQos->resource_limits.max_samples = 5;
    topicQos->resource_limits.max_instances = 1;
    topicQos->resource_limits.max_samples_per_instance = 5;

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

    // appendEntries_Listener = createDataReaderListener();

    DDS_free(topicQos);
    DDS_free(typeName);
    DDS_free(dataReaderQos);
    DDS_free(typeName);
    DDS_free(typeSupport);
    DDS_free(subscriberQos);

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
    topicQos->resource_limits.max_samples = 5;
    topicQos->resource_limits.max_instances = 1;
    topicQos->resource_limits.max_samples_per_instance = 5;

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
    DDS_free(typeName);
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
    topicQos->resource_limits.max_samples = 5;
    topicQos->resource_limits.max_instances = 1;
    topicQos->resource_limits.max_samples_per_instance = 5;

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

    // requestVoteReply_Listener = createDataReaderListener();

    DDS_free(topicQos);
    DDS_free(typeName);
    DDS_free(dataWriterQos);
    DDS_free(dataReaderQos);
    DDS_free(typeName);
    DDS_free(typeSupport);
    DDS_free(publisherQos);
    DDS_free(subscriberQos);

}

void createElectionTimerDDSFeatures() {

    electionTimer_ReadCondition = DDS_DataReader_create_readcondition(
        appendEntries_DataReader,
        DDS_NOT_READ_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE | DDS_NOT_NEW_VIEW_STATE,
        DDS_ALIVE_INSTANCE_STATE
    );
    checkHandle(electionTimer_ReadCondition, "DDS_DataReader_create_readcondition (electionTimer)");

    appendEntries_WaitSet = DDS_WaitSet__alloc();
    checkHandle(appendEntries_WaitSet, "DDS_WaitSet__alloc appendEntries");

    appendEntries_GuardList = DDS_ConditionSeq__alloc();
    checkHandle(appendEntries_GuardList, "DDS_ConditionSeq__alloc");
    appendEntries_GuardList->_maximum = 1;
    appendEntries_GuardList->_length = 0;
    appendEntries_GuardList->_release = TRUE;
    appendEntries_GuardList->_buffer = DDS_ConditionSeq_allocbuf(1);
    checkHandle(appendEntries_GuardList->_buffer, "DDS_ConditionSeq_allocbuf");

    g_status = DDS_WaitSet_attach_condition(appendEntries_WaitSet, electionTimer_ReadCondition);
    checkStatus(g_status, "DDS_WaitSet_attach_condition (appendEntries_ReadCondition)");

}

void createLeaderElectionDDSFeatures() {

    become_leader_event = DDS_GuardCondition__alloc();
    checkHandle(become_leader_event, "DDS_GuardCondition__alloc");
    become_follower_event = DDS_GuardCondition__alloc();
    checkHandle(become_follower_event, "DDS_GuardCondition__alloc");

    collectVotes_WaitSet = DDS_WaitSet__alloc();
    checkHandle(collectVotes_WaitSet, "DDS_WaitSet__alloc collect Votes");
    
    collectVotes_GuardList = DDS_ConditionSeq__alloc();
    checkHandle(collectVotes_GuardList, "DDS_ConditionSeq__alloc");
    collectVotes_GuardList->_maximum = 3;
    collectVotes_GuardList->_length = 0;
    collectVotes_GuardList->_release = TRUE;
    collectVotes_GuardList->_buffer = DDS_ConditionSeq_allocbuf(3);
    checkHandle(collectVotes_GuardList->_buffer, "DDS_ConditionSeq_allocbuf");

    requestVoteReply_ReadCondition = DDS_DataReader_create_readcondition(
        requestVoteReply_DataReader,
        DDS_NOT_READ_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE | DDS_NOT_NEW_VIEW_STATE, // TODO Validate if this is correct
        DDS_ALIVE_INSTANCE_STATE
    );
    checkHandle(requestVoteReply_ReadCondition, "DDS_DataReader_create_readcondition (requestVoteReply)");

    requestVoteReply_WaitSet = DDS_WaitSet__alloc();
    checkHandle(requestVoteReply_WaitSet, "DDS_WaitSet__alloc requestVoteReply");

    requestVoteReply_GuardList = DDS_ConditionSeq__alloc();
    checkHandle(requestVoteReply_GuardList, "DDS_ConditionSeq__alloc");
    requestVoteReply_GuardList->_maximum = 1;
    requestVoteReply_GuardList->_length = 0;
    requestVoteReply_GuardList->_release = TRUE;
    requestVoteReply_GuardList->_buffer = DDS_ConditionSeq_allocbuf(1);
    checkHandle(requestVoteReply_GuardList->_buffer, "DDS_ConditionSeq_allocbuf");

    g_status = DDS_WaitSet_attach_condition(collectVotes_WaitSet, become_leader_event);
    checkStatus(g_status, "DDS_WaitSet_attach_condition (become_leader)");
    g_status = DDS_WaitSet_attach_condition(collectVotes_WaitSet, become_follower_event);
    checkStatus(g_status, "DDS_WaitSet_attach_condition (become_follower)");
    g_status = DDS_WaitSet_attach_condition(collectVotes_WaitSet, electionTimer_ReadCondition);
    checkStatus(g_status, "DDS_WaitSet_attach_condition (append Entries readCondition)");

    g_status = DDS_WaitSet_attach_condition(requestVoteReply_WaitSet, requestVoteReply_ReadCondition);
    checkStatus(g_status, "DDS_WaitSet_attach_condition (RequestVoteReply Readcondition)");
}

void createReceiveVotesDDSFeatures() {

    requestVote_ReadCondition = DDS_DataReader_create_readcondition(
        requestVote_DataReader,
        DDS_NOT_READ_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE | DDS_NOT_NEW_VIEW_STATE, // TODO Validate if this is correct
        DDS_ALIVE_INSTANCE_STATE
    );
    checkHandle(requestVote_ReadCondition, "DDS_DataReader_create_readcondition (requestVote)");

    requestVote_WaitSet = DDS_WaitSet__alloc();
    checkHandle(requestVote_WaitSet, "DDS_WaitSet__alloc requestVote");

    requestVote_GuardList = DDS_ConditionSeq__alloc();
    checkHandle(requestVote_GuardList, "DDS_ConditionSeq__alloc");
    requestVote_GuardList->_maximum = 1;
    requestVote_GuardList->_length = 0;
    requestVote_GuardList->_release = TRUE;
    requestVote_GuardList->_buffer = DDS_ConditionSeq_allocbuf(1);
    checkHandle(requestVote_GuardList->_buffer, "DDS_ConditionSeq_allocbuf");

    g_status = DDS_WaitSet_attach_condition(requestVote_WaitSet, requestVote_ReadCondition);
    checkStatus(g_status, "DDS_WaitSet_attach_condition (requestVote_ReadCondition)");

}