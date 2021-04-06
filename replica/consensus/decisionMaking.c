#include "decisionMaking.h"

#include <stdlib.h>

#include "state/balise.h"
#include "state/train.h"
#include "state/movementAuthority.h"


bool should_brake() {
    // TODO Calculate brake curve and decide if train should brake
    return false;
}

bool process_input(const char* data, const uint8_t data_length) {
    balise_t processed_balise;

    if (data_length > 2) {
        processed_balise.internal_ID = data[0];
        processed_balise.num_balises_in_group = data[1];
        processed_balise.balise_group_ID = data[2];
    }
    // Balise group for updating position
    balise_group_t linked_balise_group;

    bool is_linked = get_balise_group_if_linked(processed_balise.balise_group_ID, &linked_balise_group);

    if (!is_linked) {
        return false;
    }

    train_state_t current_state;
    bool has_state = get_train_state(&current_state);

    if (!has_state) {
        return false;
    }

    if (abs((int)current_state.position - (int)linked_balise_group.position) > MAX_POSITION_DRIFT) {
        return false;
    }

    set_train_position(linked_balise_group.position);

    return should_brake();

}