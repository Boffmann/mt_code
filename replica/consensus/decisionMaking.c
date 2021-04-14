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

    if (current_ma.end_position < train_state->position.max_position) {
        return true;
    }

    double distance_to_ma_end = current_ma.end_position - train_state->position.max_position;

    double braking_distance_until_stop = (TRAIN_SPEED * TRAIN_SPEED) / (2 * TRAIN_DELECERATION);

    printf("Braking distance is: %lf\n", braking_distance_until_stop);

    if (distance_to_ma_end <= braking_distance_until_stop) {
        return true;
    }

    if (fabs(distance_to_ma_end - braking_distance_until_stop) > 1.0) {
        return false;
    }

    return true;
}

bool decide_should_brake(const bool is_balise_decision, const int baliseID, enum StoppedReason *reason) {

    train_state_t current_state;
    bool has_state = get_train_state(&current_state);

    if (!has_state) {
        printf("Decision making: Has not state!!\n");
        return true;
    }

    if (!is_balise_decision) {
        if (should_brake(&current_state)) {
            printf("I DECIDED THAT THE TRIAN SHOULD BRAKE\n");
            *reason = END_OF_MA;
            return true;
        } else {
            *reason = NONE;
            return false;
        }
    }

    balise_t processed_balise;
    bool is_linked = false;
    balise_t linked_balise;

    if (is_balise_decision) {
        processed_balise.ID = baliseID;

        is_linked = get_balise_if_linked(processed_balise.ID, &linked_balise);
    }
    // Balise group for updating position

    if (!is_linked) {
        printf("Encountered balise not linked ID: %d\n", processed_balise.ID);;
        *reason = BALISE_NOT_LINKED;
        return true;
    }


    if (current_state.position.min_position > linked_balise.position || current_state.position.max_position < linked_balise.position) {
        printf("Decision making: Balise not where expected!!\n");
        *reason = BALISE_NOT_WHERE_EXPECTED;
        return true;
    }

    printf("Everything fine. Calculate braking curve\n");
    
    if (should_brake(&current_state)) {
        printf("I DECIDED THAT THE TRIAN SHOULD BRAKE\n");
        *reason = END_OF_MA;
        return true;
    } else {
        *reason = NONE;
        return false;
    }

}