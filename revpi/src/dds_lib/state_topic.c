#include "state_topic.h"
#include "CheckStatus.h"
#include "datamodel.h"

on_state_data_available_t on_state_data_available_callback = NULL;

void on_state_data_available(void* listener_data, DDS_DataReader reader) {

    listener_data_t*                listener_state;
    DDS_ReturnCode_t                status;
    DDS_sequence_RevPiDDS_State*    message_seq;
    DDS_SampleInfoSeq*              message_infoSeq;
    long                            timestamp;
    (void)                          reader;             // Unused

    // Unused parameter
    listener_state = (listener_data_t*) listener_data;
    message_seq = DDS_sequence_RevPiDDS_State__alloc();
    message_infoSeq = DDS_SampleInfoSeq__alloc();

    printf("Got a state message:\n");

    status = RevPiDDS_StateDataReader_read(
        listener_state->state_data_reader,
        message_seq,
        message_infoSeq,
        DDS_LENGTH_UNLIMITED,
        DDS_ANY_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE,
        DDS_ANY_INSTANCE_STATE);
    checkStatus(status, "RevPiDDS_StateDataReader_read");

    timestamp = 0;

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

    DDS_free(message_seq);
    DDS_free(message_infoSeq);

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

dds_state_instance_t state_topic_create_new_instance(const publisher_t* publisher) {

    dds_state_instance_t new_state_instance;

    new_state_instance.message = RevPiDDS_State__alloc();
    checkHandle(new_state_instance.message, "RevPiDDS_State__alloc");
    new_state_instance.message_handle = RevPiDDS_StateDataWriter_register_instance(publisher->dds_dataWriter, new_state_instance.message);

    return new_state_instance;
}

void state_topic_dispose_instance(const publisher_t* publisher, dds_state_instance_t* state_instance) {

    // Dispose and unregister message
    DDS_ReturnCode_t status = RevPiDDS_StateDataWriter_dispose(publisher->dds_dataWriter, state_instance->message, state_instance->message_handle);
    checkStatus(status,"RevPiDDS_StateDataWriter_dispose");
    status = RevPiDDS_StateDataWriter_unregister_instance(publisher->dds_dataWriter, state_instance->message, state_instance->message_handle);
    checkStatus(status,"RevPiDDS_StateDataWriter_unregister_instance");

    DDS_free(state_instance->message);
}

void state_topic_publish(const publisher_t* publisher, dds_state_instance_t* state_instance) {

    // state_instance->message->timestamp = state_data->timestamp;
    // state_instance->message->speed = state_data->speed;

    state_instance->message_handle = RevPiDDS_StateDataWriter_register_instance(publisher->dds_dataWriter, state_instance->message);

    DDS_ReturnCode_t status = RevPiDDS_StateDataWriter_write(publisher->dds_dataWriter, state_instance->message, state_instance->message_handle);
    checkStatus(status, "RevPiDDS_StateDataWriter_write");

    // printf("Finished publishing state %ld with speed %f\n", state_data->timestamp, state_data->speed);
    // printf("Used data writer: %p\n", (void*)&publisher->dds_dataWriter);


}

bool state_topic_read(const subscriber_t* subscriber, state_t* state_data) {

    bool state_valid = false;

    DDS_sequence_RevPiDDS_State* message_sequence = DDS_sequence_RevPiDDS_State__alloc();
    checkHandle(message_sequence, "DDS_sequence_RevPiDDS_State__alloc");
    DDS_SampleInfoSeq* info_sequence = DDS_SampleInfoSeq__alloc();
    checkHandle(info_sequence, "DDS_SampleInfoSeq__alloc");

    DDS_ReturnCode_t status = RevPiDDS_StateDataReader_read(
        subscriber->dds_dataReader,
        message_sequence,
        info_sequence,
        DDS_LENGTH_UNLIMITED,
        DDS_ANY_SAMPLE_STATE,
        DDS_ANY_VIEW_STATE,
        DDS_ALIVE_INSTANCE_STATE
    );
    checkStatus(status, "RevPiDDS_StateDataReader_read");

    long timestamp = 0;
    double speed = 0.0;
    printf("Got state info of length %d\n", message_sequence->_length);

    for (DDS_unsigned_long i = 0; i < message_sequence->_length; ++i) {

        if (info_sequence->_buffer[i].valid_data) {
            state_valid = true;
            printf("Speed at: %d id %f\n", i, message_sequence->_buffer[i].speed);
            speed = message_sequence->_buffer[i].speed;
        }

    }

    // if ( message_sequence->_length > 0 ) {
    //     int message_index = message_sequence->_length - 1;
    //     if(info_sequence->_buffer[message_index].valid_data == TRUE ) {
    //         timestamp = message_sequence->_buffer[message_index].timestamp;
    //         speed = message_sequence->_buffer[message_index].speed;
    //         state_valid = true;
    //     }
    // }

    status = RevPiDDS_StateDataReader_return_loan(subscriber->dds_dataReader, message_sequence, info_sequence);
    checkStatus(status, "RevPiDDS_StateDataReader_return_loan");

    if (state_valid) {
        state_data->timestamp = timestamp;
        state_data->speed = speed;
    }

    DDS_free(message_sequence);
    DDS_free(info_sequence);

    return state_valid;

}

void state_topic_listen(listener_t* listener, on_state_data_available_t callback) {

    listener->listener_data->state_data_reader = listener->subscriber.dds_dataReader;
    listener->dds_listener->listener_data = listener->listener_data;
    listener->listener_data = listener->listener_data;
    listener->dds_listener->on_data_available = &on_state_data_available;

    DDS_StatusMask mask = DDS_DATA_AVAILABLE_STATUS | DDS_REQUESTED_DEADLINE_MISSED_STATUS;
    DDS_ReturnCode_t status = DDS_DataReader_set_listener(listener->subscriber.dds_dataReader, listener->dds_listener, mask);
    checkStatus(status, "DDS_DataReader_set_listener");

    on_state_data_available_callback = callback;
}