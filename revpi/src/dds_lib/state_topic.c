#include "state_topic.h"
#include "CheckStatus.h"
#include "datamodel.h"

on_state_data_available_t on_state_data_available_callback = NULL;

typedef struct {
    DDS_DataReader* state_data_reader;
} state_listener_data_t;

void on_state_data_available(void* listener_data, DDS_DataReader reader) {
    // Unused parameter
    (void) listener_data;
    DDS_ReturnCode_t status;
    DDS_sequence_RevPiDDS_State* message_seq = DDS_sequence_RevPiDDS_State__alloc();
    DDS_SampleInfoSeq* message_infoSeq = DDS_SampleInfoSeq__alloc();

    printf("Got a state message:\n");

    status = RevPiDDS_StateDataReader_read(reader,
        message_seq,
        message_infoSeq,
        DDS_LENGTH_UNLIMITED,
        DDS_ANY_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE,
        DDS_ANY_INSTANCE_STATE);
    checkStatus(status, "RevPiDDS_StateDataReader_read");

    long timestamp = 0;

    if ( message_seq->_length > 0 ) {
        printf( "\n=== [ListenerListener::on_data_available] - message_seq->length : %d", message_seq->_length );
        if( message_infoSeq->_buffer[0].valid_data == TRUE ) {
        printf( "\n    --- message received ---" );
        printf( "\n    userID  : %d", message_seq->_buffer[0].timestamp );
        timestamp = message_seq->_buffer[0].timestamp;
        }
    }

    status = RevPiDDS_StateDataReader_return_loan(reader, message_seq, message_infoSeq);
    checkStatus(status, "RevPiDDS_StateDataReader_return_loan");

    if (timestamp != 0) {
        printf("\n Calling callback...\n");
        state_t state_data;
        state_data.timestamp = timestamp;
        on_state_data_available_callback(&state_data);
    }

}

topic_t state_topic_create(const domain_participant_t* domain_participant, const DDS_TopicQos* topic_qos) {

    DDS_ReturnCode_t status;

    // Register Topic's name in DDS Domain
    DDS_TypeSupport message_type_support = RevPiDDS_StateTypeSupport__alloc();
    checkHandle(message_type_support, "RevPiDDS_StateTypeSupport__alloc");
    char* type_name = RevPiDDS_StateTypeSupport_get_type_name(message_type_support);

    status = RevPiDDS_StateTypeSupport_register_type(message_type_support, domain_participant->dds_domainParticipant, type_name);
    checkStatus(status,"RevPiDDS_StateTypeSupport_register_type");

    DDS_free(type_name);

    char* message_type_name = RevPiDDS_StateTypeSupport_get_type_name(message_type_support);

    DDS_free(message_type_support);

    topic_t new_topic = topic_create_new(
        domain_participant->dds_domainParticipant,
        "State_Topic",
        message_type_name,
        topic_qos
    );

    DDS_free(message_type_name);

    return new_topic;
}

void state_topic_publish(const publisher_t* publisher, const state_t* state_data) {

    RevPiDDS_State *message = RevPiDDS_State__alloc();
    checkHandle(message, "RevPiDDS_State__alloc");
    message->timestamp = state_data->timestamp;
    DDS_InstanceHandle_t message_handle = RevPiDDS_StateDataWriter_register_instance(publisher->dds_dataWriter, message);

    DDS_ReturnCode_t status = RevPiDDS_StateDataWriter_write(publisher->dds_dataWriter, message, message_handle);
    checkStatus(status, "RevPiDDS_StateDataWriter_write");

    printf("Finished publishing %ld\n", state_data->timestamp);
    printf("Used data writer: %p\n", (void*)&publisher->dds_dataWriter);

    // Dispose and unregister message
    status = RevPiDDS_StateDataWriter_dispose(publisher->dds_dataWriter, message, message_handle);
    checkStatus(status,"RevPiDDS_StateDataWriter_dispose");
    status = RevPiDDS_StateDataWriter_unregister_instance(publisher->dds_dataWriter, message, message_handle);
    checkStatus(status,"RevPiDDS_StateDataWriter_unregister_instance");

    DDS_free(message);

}

void state_topic_listen(listener_t* listener, on_state_data_available_t callback) {

    state_listener_data_t* listener_data = malloc(sizeof(state_listener_data_t));
    checkHandle(listener_data, "malloc");
    listener_data->state_data_reader = &listener->dds_dataReader;
    listener->dds_listener->listener_data = listener_data;
    listener->dds_listener->on_data_available = &on_state_data_available;

    DDS_StatusMask mask = DDS_DATA_AVAILABLE_STATUS | DDS_REQUESTED_DEADLINE_MISSED_STATUS;
    DDS_ReturnCode_t status = DDS_DataReader_set_listener(listener->dds_dataReader, listener->dds_listener, mask);
    checkStatus(status, "DDS_DataReader_set_listener");


    on_state_data_available_callback = callback;
}