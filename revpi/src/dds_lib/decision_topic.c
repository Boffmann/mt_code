#include "decision_topic.h"
#include "CheckStatus.h"
#include "datamodel.h"

on_decision_data_available_t on_decision_data_available_callback = NULL;

typedef struct {
    DDS_DataReader* decision_data_reader;
} decision_listener_data_t;

// TODO Also include is_deciusion_valid boolean in callback
void on_decision_data_available(void* listener_data, DDS_DataReader reader) {
    // Unused parameter
    (void) listener_data;
    DDS_ReturnCode_t status;
    DDS_sequence_RevPiDDS_Decision* message_seq = DDS_sequence_RevPiDDS_Decision__alloc();
    DDS_SampleInfoSeq* message_infoSeq = DDS_SampleInfoSeq__alloc();

    printf("Got a decision message:\n");

    status = RevPiDDS_DecisionDataReader_read(reader,
        message_seq,
        message_infoSeq,
        DDS_LENGTH_UNLIMITED,
        DDS_ANY_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE,
        DDS_ANY_INSTANCE_STATE);
    checkStatus(status, "RevPiDDS_DecisionDataReader_read");

    long decisionid = 0;

    if ( message_seq->_length > 0 ) {
        if( message_infoSeq->_buffer[0].valid_data == TRUE ) {
            decisionid = message_seq->_buffer[0].decisionID;
        }
    }

    status = RevPiDDS_DecisionDataReader_return_loan(reader, message_seq, message_infoSeq);
    checkStatus(status, "RevPiDDS_DecisionDataReader_return_loan");


    if (decisionid != 0) {
        printf("\n Calling decision callback...\n");
        decision_t decision_data;
        decision_data.decision_id = decisionid;
        on_decision_data_available_callback(&decision_data);
    }

}

topic_t decision_topic_create(const domain_participant_t* domain_participant, const DDS_TopicQos* topic_qos) {

    DDS_ReturnCode_t status;

    // Register Topic's name in DDS Domain
    DDS_TypeSupport message_type_support = RevPiDDS_DecisionTypeSupport__alloc();
    checkHandle(message_type_support, "RevPiDDS_DecisionTypeSupport__alloc");
    char* type_name = RevPiDDS_DecisionTypeSupport_get_type_name(message_type_support);

    status = RevPiDDS_DecisionTypeSupport_register_type(message_type_support, domain_participant->dds_domainParticipant, type_name);
    checkStatus(status,"RevPiDDS_DecisionTypeSupport_register_type");

    DDS_free(type_name);

    char* message_type_name = RevPiDDS_DecisionTypeSupport_get_type_name(message_type_support);

    DDS_free(message_type_support);

    topic_t new_topic = topic_create_new(
        domain_participant->dds_domainParticipant,
        "Decision_Topic",
        message_type_name,
        topic_qos
    );

    DDS_free(message_type_name);

    return new_topic;
}

void decision_topic_publish(const publisher_t* publisher, const decision_t* decision_data) {

    RevPiDDS_Decision *message = RevPiDDS_Decision__alloc();
    checkHandle(message, "RevPiDDS_Decision__alloc");
    message->decisionID = decision_data->decision_id;
    DDS_InstanceHandle_t message_handle = RevPiDDS_DecisionDataWriter_register_instance(publisher->dds_dataWriter, message);

    DDS_ReturnCode_t status = RevPiDDS_DecisionDataWriter_write(publisher->dds_dataWriter, message, message_handle);
    checkStatus(status, "RevPiDDS_DecisionDataWriter_write");

    printf("Finished publishing %ld\n", decision_data->decision_id);
    printf("Used data writer: %p\n", (void*)&publisher->dds_dataWriter);

    // Dispose and unregister message
    status = RevPiDDS_DecisionDataWriter_dispose(publisher->dds_dataWriter, message, message_handle);
    checkStatus(status,"RevPiDDS_DecisionDataWriter_dispose");
    status = RevPiDDS_DecisionDataWriter_unregister_instance(publisher->dds_dataWriter, message, message_handle);
    checkStatus(status,"RevPiDDS_DecisionDataWriter_unregister_instance");

    DDS_free(message);

}

bool decision_topic_read(const subscriber_t* subscriber, decision_t* decision_data) {

    bool decision_valid = false;

    DDS_sequence_RevPiDDS_Decision* message_sequence = DDS_sequence_RevPiDDS_Decision__alloc();
    checkHandle(message_sequence, "DDS_sequence_RevPiDDS_Decision__alloc");
    DDS_SampleInfoSeq* info_sequence = DDS_SampleInfoSeq__alloc();
    checkHandle(info_sequence, "DDS_SampleInfoSeq__alloc");

    DDS_ReturnCode_t status = RevPiDDS_DecisionDataReader_take(
        subscriber->dds_dataReader,
        message_sequence,
        info_sequence,
        DDS_LENGTH_UNLIMITED,
        DDS_ANY_SAMPLE_STATE,
        DDS_ANY_VIEW_STATE,
        DDS_ALIVE_INSTANCE_STATE
    );
    checkStatus(status, "RevPiDDS_DecisionDataReader_take");

    long decisionid = 0;

    if ( message_sequence->_length > 0 ) {
        if(info_sequence->_buffer[0].valid_data == TRUE ) {
            decisionid = message_sequence->_buffer[0].decisionID;
            decision_valid = true;
        }
    }

    status = RevPiDDS_DecisionDataReader_return_loan(subscriber->dds_dataReader, message_sequence, info_sequence);
    checkStatus(status, "RevPiDDS_DecisionDataReader_return_loan");

    if (decision_valid) {
        decision_data->decision_id = decisionid;
    }

    return decision_valid;

}

void decision_topic_listen(listener_t* listener, on_decision_data_available_t callback) {

    decision_listener_data_t* listener_data = malloc(sizeof(decision_listener_data_t));
    checkHandle(listener_data, "malloc");
    listener_data->decision_data_reader = &listener->subscriber.dds_dataReader;
    listener->dds_listener->listener_data = listener_data;
    listener->dds_listener->on_data_available = &on_decision_data_available;

    DDS_StatusMask mask = DDS_DATA_AVAILABLE_STATUS | DDS_REQUESTED_DEADLINE_MISSED_STATUS;
    DDS_ReturnCode_t status = DDS_DataReader_set_listener(listener->subscriber.dds_dataReader, listener->dds_listener, mask);
    checkStatus(status, "DDS_DataReader_set_listener");

    on_decision_data_available_callback = callback;
}