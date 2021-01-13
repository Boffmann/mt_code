#include "task_topic.h"
#include "CheckStatus.h"
#include "datamodel.h"

on_task_data_available_t on_task_data_available_callback = NULL;

void on_task_data_available(void* listener_data, DDS_DataReader reader) {
    // Unused parameter
    (void) listener_data;
    DDS_ReturnCode_t status;
    DDS_sequence_RevPiDDS_Tasks* message_seq = DDS_sequence_RevPiDDS_Tasks__alloc();
    DDS_SampleInfoSeq* message_infoSeq = DDS_SampleInfoSeq__alloc();

    printf("Got a message:\n");

    status = RevPiDDS_TasksDataReader_read(reader,
        message_seq,
        message_infoSeq,
        DDS_LENGTH_UNLIMITED,
        DDS_ANY_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE,
        DDS_ANY_INSTANCE_STATE);
    checkStatus(status, "RevPiDDS_TasksDataReader_read");

    long taskid = 0;

    if ( message_seq->_length > 0 ) {
        printf( "\n=== [ListenerListener::on_data_available] - message_seq->length : %d", message_seq->_length );
        if( message_infoSeq->_buffer[0].valid_data == TRUE ) {
        printf( "\n    --- message received ---" );
        printf( "\n    userID  : %d", message_seq->_buffer[0].taskID );
        printf( "\n    Message : \"%s\"", message_seq->_buffer[0].message );
        taskid = message_seq->_buffer[0].taskID;
        }
    }


    status = RevPiDDS_TasksDataReader_return_loan(reader, message_seq, message_infoSeq);
    checkStatus(status, "RevPiDDS_TasksDataReader_return_loan");


    if (taskid != 0) {
        printf("\n Calling callback...\n");
        task_t task_data;
        task_data.task_id = taskid;
        on_task_data_available_callback(&task_data);
    }

}

topic_t tasks_topic_create(const domain_participant_t* domain_participant) {

    DDS_ReturnCode_t status;

    // Register Topic's name in DDS Domain
    DDS_TypeSupport message_type_support = RevPiDDS_TasksTypeSupport__alloc();
    checkHandle(message_type_support, "RevPiDDS_TasksTypeSupport__alloc");
    // registerMessageType(message_type_support);
    char* type_name = RevPiDDS_TasksTypeSupport_get_type_name(message_type_support);

    status = RevPiDDS_TasksTypeSupport_register_type(message_type_support, domain_participant->dds_domainParticipant, type_name);
    checkStatus(status,"RevPiDDS_TasksTypeSupport_register_type");

    DDS_free(type_name);

    char* message_type_name = RevPiDDS_TasksTypeSupport_get_type_name(message_type_support);
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
        "Tasks_Topic",
        message_type_name,
        topic_qos
    );

    DDS_free(message_type_name);
    DDS_free(topic_qos);

    return new_topic;

}

void task_topic_listen(listener_t* listener, on_task_data_available_t callback) {

    task_listener_data_t* listener_data = malloc(sizeof(task_listener_data_t));
    checkHandle(listener_data, "malloc");
    listener_data->task_data_reader = &listener->dds_dataReader;
    listener->dds_listener->listener_data = listener_data;
    listener->dds_listener->on_data_available = on_task_data_available;

    on_task_data_available_callback = callback;

}