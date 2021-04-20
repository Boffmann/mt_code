#include "train.h"

#include <sys/time.h>

#include "datamodel.h"
#include "DDSStateManager.h"
#include "evaluation/evaluator.h"
#include "consensus/replica.h"

void initialize_train_state() {
    DDS_ReturnCode_t status;
    RevPiDDS_TrainState *newTrainState_message = RevPiDDS_TrainState__alloc();

    struct timeval time_now;
    gettimeofday(&time_now, NULL);
    unsigned long long now_microseconds = time_now.tv_sec * 1000000L + time_now.tv_usec;

    printf("Initializing train state\n");
    newTrainState_message->position = 0;
    newTrainState_message->max_position = 0;
    newTrainState_message->min_position = 0;
    newTrainState_message->speed = TRAIN_SPEED;
    newTrainState_message->is_driving = true;
    newTrainState_message->lastUpdateTime = now_microseconds;
    evaluator_register_message_send(this_replica->ID, "TrainState", this_replica->role == LEADER);
    status = RevPiDDS_TrainStateDataWriter_write(trainState_DataWriter, newTrainState_message, DDS_HANDLE_NIL);
    checkStatus(status, "RevPiDDS_TrainStateDataWriter write Initial");

    DDS_free(newTrainState_message);

}

void update_train_position() {
    DDS_ReturnCode_t status;
    RevPiDDS_TrainState *newTrainState_message = RevPiDDS_TrainState__alloc();
    train_state_t last_state;

    struct timeval time_now;
    gettimeofday(&time_now, NULL);
    unsigned long long now_microseconds = time_now.tv_sec * 1000000L + time_now.tv_usec;

    bool has_state = get_train_state(&last_state);


    if (has_state) {

        if (last_state.is_driving) {
            unsigned long long elapsed_time = now_microseconds - last_state.lastUpdateTime;
            double random_sensor_drift = ((double) rand() * MAX_POS_INACCURACY ) / (double) RAND_MAX;
            double traveled_distance = TRAIN_SPEED * ((double) elapsed_time / 1000000.0);
            double new_position = last_state.position.position + traveled_distance;
            double new_max_position = last_state.position.max_position + traveled_distance + traveled_distance * random_sensor_drift;
            double new_min_position = last_state.position.min_position + traveled_distance - traveled_distance * random_sensor_drift;

            printf("Last Position at: %lf\n", last_state.position.position);
            printf("elapsed time: %lld\n", elapsed_time);
            printf("New MIN Position at: %lf\n", new_min_position);
            printf("New Position at: %lf\n", new_position);
            printf("New MAX Position at: %lf\n", new_max_position);

            newTrainState_message->position = new_position;
            newTrainState_message->max_position = new_max_position;
            newTrainState_message->min_position = new_min_position;
            newTrainState_message->speed = TRAIN_SPEED;
            newTrainState_message->is_driving = last_state.is_driving;
            newTrainState_message->lastUpdateTime = now_microseconds;
            evaluator_register_message_send(this_replica->ID, "TrainState", this_replica->role == LEADER);
            status = RevPiDDS_TrainStateDataWriter_write(trainState_DataWriter, newTrainState_message, DDS_HANDLE_NIL);
            checkStatus(status, "RevPiDDS_TrainStateDataWriter write Initial");
        }
    }

    DDS_free(newTrainState_message);

}

void set_train_driving(const bool is_driving) {
    DDS_ReturnCode_t status;
    RevPiDDS_TrainState *newTrainState_message = RevPiDDS_TrainState__alloc();
    train_state_t last_state;

    bool has_state = get_train_state(&last_state);

    if (has_state) {

        newTrainState_message->position = last_state.position.position;
        newTrainState_message->max_position = last_state.position.max_position;
        newTrainState_message->min_position = last_state.position.min_position;
        newTrainState_message->speed = last_state.speed;
        newTrainState_message->is_driving = is_driving;
        newTrainState_message->lastUpdateTime = last_state.lastUpdateTime;
        evaluator_register_message_send(this_replica->ID, "TrainState", this_replica->role == LEADER);
        status = RevPiDDS_TrainStateDataWriter_write(trainState_DataWriter, newTrainState_message, DDS_HANDLE_NIL);
        checkStatus(status, "RevPiDDS_TrainStateDataWriter write Initial");
    }
    DDS_free(newTrainState_message);
}

void set_train_position(const double position) {
    DDS_ReturnCode_t status;
    RevPiDDS_TrainState *newTrainState_message = RevPiDDS_TrainState__alloc();
    train_state_t last_state;

    bool has_state = get_train_state(&last_state);

    if (has_state) {
        struct timeval time_now;
        gettimeofday(&time_now, NULL);
        unsigned long long now_microseconds = time_now.tv_sec * 1000000L + time_now.tv_usec;

        newTrainState_message->position = position;
        newTrainState_message->max_position = position;
        newTrainState_message->min_position = position;
        newTrainState_message->speed = last_state.speed;
        newTrainState_message->is_driving = last_state.is_driving;
        newTrainState_message->lastUpdateTime = now_microseconds;
        evaluator_register_message_send(this_replica->ID, "TrainState", this_replica->role == LEADER);
        status = RevPiDDS_TrainStateDataWriter_write(trainState_DataWriter, newTrainState_message, DDS_HANDLE_NIL);
        checkStatus(status, "RevPiDDS_TrainStateDataWriter write Initial");
        DDS_free(newTrainState_message);
    }

}

bool get_train_state(train_state_t* train_state) {

    DDS_ReturnCode_t status;
    DDS_sequence_RevPiDDS_TrainState msgSeq  = {0, 0, DDS_OBJECT_NIL, FALSE};
    DDS_SampleInfoSeq                   infoSeq = {0, 0, DDS_OBJECT_NIL, FALSE};

    status = RevPiDDS_TrainStateDataReader_read (
        trainState_DataReader,
        &msgSeq,
        &infoSeq,
        1,
        DDS_READ_SAMPLE_STATE | DDS_NOT_READ_SAMPLE_STATE,
        DDS_NEW_VIEW_STATE | DDS_NOT_NEW_VIEW_STATE,
        DDS_ALIVE_INSTANCE_STATE
    );
    checkStatus(status, "RevPiDDS_BaliseGroupDataReader_read");

    if (msgSeq._length > 0) {
        if (infoSeq._buffer[0].valid_data) {
            // printf("Reading Train State!----------------------------------------------------------------------\n");
            // evaluator_register_message_received(this_replica->ID, "TrainState", this_replica->role == LEADER);
            train_state->position.position = msgSeq._buffer[0].position;
            train_state->position.max_position = msgSeq._buffer[0].max_position;
            train_state->position.min_position = msgSeq._buffer[0].min_position;
            train_state->speed = msgSeq._buffer[0].speed;
            train_state->is_driving = msgSeq._buffer[0].is_driving;
            train_state->lastUpdateTime = msgSeq._buffer[0].lastUpdateTime;
            status = RevPiDDS_TrainStateDataReader_return_loan(trainState_DataReader, &msgSeq, &infoSeq);
            checkStatus(status, "RevPiDDS_TrainStateDataReader_return_loan");
            return true;
        }
    }

    status = RevPiDDS_TrainStateDataReader_return_loan(trainState_DataReader, &msgSeq, &infoSeq);
    checkStatus(status, "RevPiDDS_TrainStateDataReader_return_loan");

    return false;
}