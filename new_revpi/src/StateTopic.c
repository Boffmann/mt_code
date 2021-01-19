#include "StateTopic.h"
#include "DDSEntitiesManager.h"
#include "CheckStatus.h"

char* g_StateTypeName = DDS_OBJECT_NIL;
DDS_TypeSupport g_StateTypeSupport = DDS_OBJECT_NIL;
DDS_Topic g_StateTopic = DDS_OBJECT_NIL;

void registerMessageType(DDS_TypeSupport typeSupport) {

    char* typeName = RevPiDDS_StateTypeSupport_get_type_name(typeSupport);

    g_status = RevPiDDS_StateTypeSupport_register_type(typeSupport, g_domainParticipant, typeName);
    checkStatus(g_status, "RevPiDDS_StateTypeSupport_register_type");

    DDS_free(typeName);

}

void stateTopic_create(const char* topicName) {
    // Register the Topic's type in the DDS Domain.
    g_StateTypeSupport = RevPiDDS_StateTypeSupport__alloc();
    checkHandle(g_StateTypeSupport, "RevPiDDS_StateTypeSupport__alloc");
    registerMessageType(g_StateTypeSupport);

    DDS_TopicQos* topicQos = DDS_TopicQos__alloc();
    checkHandle(topicQos, "DDS_TopicQos__alloc");
    g_status = DDS_DomainParticipant_get_default_topic_qos(g_domainParticipant, topicQos);
    checkStatus(g_status, "DDS_DomainParticipant_get_default_topic_qos");
    topicQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    topicQos->durability.kind = DDS_TRANSIENT_DURABILITY_QOS;

    // DeadlineQoSPolicy : period used to trigger the listener
    // (on_requested_deadline_missed)
    topicQos->deadline.period.nanosec = 0;
    topicQos->deadline.period.sec = 1;


    // Create the Topic's in the DDS Domain.
    g_StateTypeName = RevPiDDS_StateTypeSupport_get_type_name(g_StateTypeSupport);
    g_StateTopic = createTopic(topicName, g_StateTypeName, topicQos);
    DDS_free(g_StateTypeName);
    DDS_free(g_StateTypeSupport);
}

void stateTopic_write(DDS_DataWriter dataWriter) {
    RevPiDDS_State* stateMessage;

    stateMessage = RevPiDDS_State__alloc();
    stateMessage->timestamp = 0;
    stateMessage->speed = 12.0;

    g_status = RevPiDDS_StateDataWriter_write(dataWriter, stateMessage, DDS_HANDLE_NIL);
    checkStatus(g_status, "RevPiDDS_StateDataWriter_write");

    DDS_free(stateMessage);
}