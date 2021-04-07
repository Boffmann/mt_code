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

    if (data_length > 1) {
        processed_balise.ID = data[0];
        processed_balise.position = data[1];
    }
    // Balise group for updating position
    balise_t linked_balise;

    bool is_linked = get_balise_if_linked(processed_balise.ID, &linked_balise);

    if (!is_linked) {
        return false;
    }

    train_state_t current_state;
    bool has_state = get_train_state(&current_state);

    if (!has_state) {
        return false;
    }

    if (current_state.position.min_position > linked_balise.position || current_state.position.max_position < linked_balise.position) {
        return false;
    }

    return should_brake();

}