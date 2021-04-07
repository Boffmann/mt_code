#ifndef __REVPI_BALISE_H__
#define __REVPI_BALISE_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {

    uint8_t ID;
    uint32_t position;

} balise_t;

bool get_balise_if_linked(const uint8_t ID, balise_t* balise);

void add_linked_balise(const balise_t* balise);

#endif