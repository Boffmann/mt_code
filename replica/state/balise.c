#include "balise.h"

#include "datamodel.h"
#include "DDSStateManager.h"

void add_linked_balise(const balise_t* balise);

bool get_balise_if_linked(const uint8_t ID, balise_t* balise) {
    DDS_ReturnCode_t status;
    DDS_sequence_RevPiDDS_LinkedBalises msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    uint8_t balise_id;


    status = RevPiDDS_LinkedBalisesDataReader_read (
        linkedBalises_DataReader,
        &msgSeq,
        &infoSeq,
        DDS_LENGTH_UNLIMITED,
        DDS_READ_SAMPLE_STATE | DDS_NOT_READ_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE | DDS_NOT_NEW_VIEW_STATE,
        DDS_ALIVE_INSTANCE_STATE
    );
    checkStatus(status, "RevPiDDS_LinkedBalisesDataReader_read");


    if (msgSeq._length > 0) {
        for (DDS_unsigned_long i = 0; i < msgSeq._length; ++i) {
            if (infoSeq._buffer[i].valid_data) {
                balise_id = msgSeq._buffer[i].ID;
                printf("Found this balise in linked list: ID: %d Position: %d\n", balise_id, msgSeq._buffer[i].position);

                if (balise_id == ID) {
                    printf("The balise with ID %d is linked\n", balise_id);
                    balise->ID = balise_id;
                    balise->position = msgSeq._buffer[i].position;
                    status = RevPiDDS_LinkedBalisesDataReader_return_loan(linkedBalises_DataReader, &msgSeq, &infoSeq);
                    checkStatus(status, "RevPiDDS_LinkedBaliseDataReader_return_loan");
                    return true;
                }
            }
        }
    }

    status = RevPiDDS_LinkedBalisesDataReader_return_loan(linkedBalises_DataReader, &msgSeq, &infoSeq);
    checkStatus(status, "RevPiDDS_LinkedBaliseDataReader_return_loan");

    return false;
}

bool set_linked_balises(const DDS_sequence_long input_data) {

    DDS_ReturnCode_t status;
    DDS_sequence_RevPiDDS_LinkedBalises msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_InstanceHandle_t handle;

    if (input_data._buffer[1] % 2 != 0) {
        return false;
    }

    status = RevPiDDS_LinkedBalisesDataReader_read (
        linkedBalises_DataReader,
        &msgSeq,
        &infoSeq,
        DDS_LENGTH_UNLIMITED,
        DDS_READ_SAMPLE_STATE | DDS_NOT_READ_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE | DDS_NOT_NEW_VIEW_STATE,
        DDS_ALIVE_INSTANCE_STATE
    );
    checkStatus(status, "RevPiDDS_LinkedBalisesDataReader_read");


    if (msgSeq._length > 0) {
        for (DDS_unsigned_long i = 0; i < msgSeq._length; ++i) {
            if (infoSeq._buffer[i].valid_data) {
                handle = RevPiDDS_LinkedBalisesDataWriter_lookup_instance(
                    linkedBalises_DataWriter,
                    &msgSeq._buffer[i]
                );
                status = RevPiDDS_LinkedBalisesDataWriter_dispose(
                    linkedBalises_DataWriter,
                    &msgSeq._buffer[i],
                    handle);
                checkStatus(status, "Input_DataWriter Dispose input");
            }
        }
    }

    status = RevPiDDS_LinkedBalisesDataReader_return_loan(linkedBalises_DataReader, &msgSeq, &infoSeq);
    checkStatus(status, "RevPiDDS_LinkedBaliseDataReader_return_loan");

    int num_linked_balises = input_data._buffer[1] / 2;
    for (int i = 0; i < num_linked_balises; ++i) {
        balise_t balise;
        balise.ID = input_data._buffer[2 + 2 * i];
        balise.position = input_data._buffer[2 + 2 * i + 1];
        printf("Add a balise %d at position: %d\n", balise.ID, balise.position);
        add_linked_balise(&balise);
    }

    return true;
}

void add_linked_balise(const balise_t* balise) {
    RevPiDDS_LinkedBalises *linked_balise_message = DDS_OBJECT_NIL;
    DDS_ReturnCode_t status;
    DDS_InstanceHandle_t linked_balise_instance;

    linked_balise_message = RevPiDDS_LinkedBalises__alloc();
    linked_balise_message->ID = balise->ID;
    linked_balise_message->position = balise->position;

    linked_balise_instance = RevPiDDS_LinkedBalisesDataWriter_register_instance(linkedBalises_DataWriter, linked_balise_message);

    status = RevPiDDS_LinkedBalisesDataWriter_write(linkedBalises_DataWriter, linked_balise_message, linked_balise_instance);
    checkStatus(status, "RevPiDDS_LinkedBalisesDataWriter_write LinkedBalise");

    // status = RevPiDDS_LinkedBalisesDataWriter_unregister_instance(linkedBalises_DataWriter, linked_balise_message, linked_balise_instance);
    // checkStatus(status, "RevPiDDS_LinkedBalisesDataWriter_unregister_instance (linked balise)");

    DDS_free(linked_balise_message);
}