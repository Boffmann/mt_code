#ifndef __REVPI_BALISE_H__
#define __REVPI_BALISE_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {

    uint8_t internal_ID;
    uint8_t num_balises_in_group;
    uint8_t balise_group_ID;

} balise_t;

typedef struct {

    uint8_t ID;
    uint32_t position;

} balise_group_t;

balise_group_t* linked_balise_groups;

bool get_balise_group_if_linked(const uint8_t ID, balise_group_t* group);

#endif