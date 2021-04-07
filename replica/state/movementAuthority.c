#include "movementAuthority.h"

#include "datamodel.h"
#include "DDSStateManager.h"

void set_movement_authority(const movement_authority_t* ma) {
    DDS_ReturnCode_t status;
    RevPiDDS_MovementAuthority *movementAuthority_message = RevPiDDS_MovementAuthority__alloc();

    movementAuthority_message->start_position = ma->start_position;
    movementAuthority_message->end_position = ma->end_position;
    status = RevPiDDS_MovementAuthorityDataWriter_write(movementAuthority_DataWriter, movementAuthority_message, DDS_HANDLE_NIL);
    printf("Added a MA to topic: Positions: %d %d\n", ma->start_position, ma->end_position);
    checkStatus(status, "RevPiDDS_TrainStateDataWriter write Initial");
    DDS_free(movementAuthority_message);
}

bool get_movement_authority(movement_authority_t* ma) {
    DDS_ReturnCode_t status;
    DDS_sequence_RevPiDDS_MovementAuthority msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};

    status = RevPiDDS_MovementAuthorityDataReader_read (
        movementAuthority_DataReader,
        &msgSeq,
        &infoSeq,
        1,
        DDS_READ_SAMPLE_STATE | DDS_NOT_READ_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE | DDS_NOT_NEW_VIEW_STATE,
        DDS_ALIVE_INSTANCE_STATE
    );
    checkStatus(status, "RevPiDDS_movementAuthorityDataReader_read");

    if (msgSeq._length > 0) {
        if (infoSeq._buffer[0].valid_data) {
            ma->start_position = msgSeq._buffer[0].start_position;
            ma->end_position = msgSeq._buffer[0].end_position;
            // printf("Got a MA with positions: %d %d\n", ma->start_position, ma->end_position);
            return true;
        }
    }
    
    // printf("Sad but true. No MA found\n");
    return false;
}