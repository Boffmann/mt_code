#include "decisionMaking.h"

#include <stdlib.h>
#include <stdio.h>

#include "state/balise.h"
#include "state/train.h"
#include "state/movementAuthority.h"


bool should_brake(const train_state_t* train_state) {

    movement_authority_t current_ma;
    bool has_ma = get_movement_authority(&current_ma);

    if (!has_ma) {
        return true;
    }

    double distance_to_ma_end = current_ma.end_position - train_state->position.max_position;

    double braking_distance_until_stop = (TRAIN_SPEED * TRAIN_SPEED) / (2 * TRAIN_DELECERATION);

    printf("Braking distance is: %lf\n", braking_distance_until_stop);

    if (fabs(distance_to_ma_end - braking_distance_until_stop) > 0.5) {
        return false;
    }

    return true;
}

bool decide_should_brake(DDS_sequence_long *data) {

    train_state_t current_state;
    bool has_state = get_train_state(&current_state);

    if (!has_state) {
        printf("Decision making: Has not state!!\n");
        return true;
    }

    if (data == NULL) {
        return should_brake(&current_state);
    }

    balise_t processed_balise;

    uint8_t data_length = data->_buffer[1];

    if (data_length > 0) {
        processed_balise.ID = data->_buffer[2];
    }
    // Balise group for updating position
    balise_t linked_balise;

    bool is_linked = get_balise_if_linked(processed_balise.ID, &linked_balise);

    if (!is_linked) {
        printf("Encountered balise not linked ID: %d\n", processed_balise.ID);;
        return true;
    }


    if (current_state.position.min_position > linked_balise.position || current_state.position.max_position < linked_balise.position) {
        printf("Decision making: Balise not where expected!!\n");
        return true;
    }

    printf("Everything fine. Calculate braking curve\n");
    return should_brake(&current_state);

}