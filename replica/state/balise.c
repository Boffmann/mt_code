#include "balise.h"

#include "datamodel.h"
#include "DDSStateManager.h"

bool get_balise_group_if_linked(const uint8_t ID, balise_group_t* group) {
    DDS_ReturnCode_t status;
    DDS_sequence_RevPiDDS_BaliseGroup msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    uint8_t balise_group_id;

    status = RevPiDDS_BaliseGroupDataReader_read (
        baliseGroup_DataReader,
        &msgSeq,
        &infoSeq,
        DDS_LENGTH_UNLIMITED,
        DDS_READ_SAMPLE_STATE | DDS_NOT_READ_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE | DDS_NOT_NEW_VIEW_STATE,
        DDS_ALIVE_INSTANCE_STATE
    );
    checkStatus(status, "RevPiDDS_BaliseGroupDataReader_read");

    if (msgSeq._length > 0) {
        for (DDS_unsigned_long i = 0; i < msgSeq._length; ++i) {
            if (infoSeq._buffer[i].valid_data) {
                balise_group_id = msgSeq._buffer[i].ID;

                if (balise_group_id == ID) {
                    group->ID = balise_group_id;
                    group->position = msgSeq._buffer[i].position;
                    return true;
                }
            }
        }
    }

    return false;
}