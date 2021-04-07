#include "balise.h"

#include "datamodel.h"
#include "DDSStateManager.h"

bool get_balise_if_linked(const uint8_t ID, balise_t* balise) {
    DDS_ReturnCode_t status;
    DDS_sequence_RevPiDDS_LinkedBalises msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    uint8_t balise_group_id;

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
                balise_group_id = msgSeq._buffer[i].ID;

                if (balise_group_id == ID) {
                    balise->ID = balise_group_id;
                    balise->position = msgSeq._buffer[i].position;
                    return true;
                }
            }
        }
    }

    return false;
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

    DDS_free(linked_balise_message);
}