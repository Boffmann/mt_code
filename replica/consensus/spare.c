#include "spare.h"

#include "datamodel.h"
#include "DDSConsensusManager.h"
#include "dds_dcps.h"
#include "replica.h"

bool activate_when_promted() {
    DDS_sequence_RevPiDDS_ActivateSpare msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};
    int received_Term = this_replica->current_term;
    bool should_activate = false;
    DDS_ReturnCode_t status;

    printf("Reading from ActivateSpareTopic\n");
    status = RevPiDDS_ActivateSpareDataReader_take_w_condition(
        activateSpare_DataReader,
        &msgSeq,
        &infoSeq,
        DDS_LENGTH_UNLIMITED,
        activateSpare_ReadCondition
    );
    checkStatus(status, "RevPiDDS_ActivateSpareDataReader_take");

    if (msgSeq._length > 0) {
    evaluator_register_message_received(this_replica->ID, "ActivateSpare", this_replica->role == LEADER);

        for (DDS_unsigned_long i = 0; i < msgSeq._length; ++i) {
            if (infoSeq._buffer[i].valid_data) {
                if (msgSeq._buffer[i].term >= received_Term) {
                    received_Term = msgSeq._buffer[i].term;
                    should_activate = msgSeq._buffer[i].activate;
                }
            }
        }
    } else {
        printf("Got nothing new on activateSpare Topic\n");
        return this_replica->role == FOLLOWER;
    }

    printf("Should activate: %d in term %d\n", should_activate, received_Term);
    if (should_activate) {
        become_follower(received_Term);
        return true;
    } else {
        become_spare();
        return false;
    }
}

bool spare_wait_until_activated() {

    if (this_replica->role != SPARE) {
        printf("This replica is no spare. Would make no sense to wait until activated\n");
        return true;
    }

    DDS_ReturnCode_t status;
    DDS_Duration_t timeout = DDS_DURATION_INFINITE;

    printf("Wait until activate event gets published to activateSpare topic\n");
    status = DDS_WaitSet_wait(activateSpare_WaitSet, activateSpare_GuardList, &timeout);

    if (this_replica->role != SPARE) {
        printf("This replica is no spare anymore. Would make no sense to wait until activated\n");
        return true;
    }

    if (status != DDS_RETCODE_TIMEOUT) {
        return activate_when_promted();
    }
    
    printf("This should be unreachable\n");
    return false;
}