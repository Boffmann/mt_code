#ifndef __REVPI_SCENARIO_H__
#define __REVPI_SCENARIO_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "movementAuthority.h"

typedef enum { NONE, SCENARIO, MA, BALISES, BALISE } parse_keys;
typedef enum { START, KEY, VALUE, STOP} parse_states;

typedef struct {

    uint32_t id;
    uint32_t position;
    uint32_t linked_position;
    bool linked;
    
} balise_t;

typedef struct {
    balise_t* array;
    size_t used;
    size_t size;
} balise_array_t;

// typedef struct {
    
//     uint32_t start_pos;
//     uint32_t end_pos;

// } movement_authority_t;

typedef struct {

    uint16_t num_linked_balises;
    balise_array_t balises;
    movement_authority_t movement_authority;

} scenario_t;

void baliseArray_add_balise(balise_array_t* array, balise_t balise);

bool create_scenario_from(const char* scenario_path, scenario_t* scenario);
void scenario_cleanup(scenario_t *scenario);
balise_array_t* scenario_get_linked_balises(const scenario_t* const scenario);

#endif