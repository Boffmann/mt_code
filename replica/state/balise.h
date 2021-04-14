#ifndef __REVPI_BALISE_H__
#define __REVPI_BALISE_H__

#include <stdint.h>
#include <stdbool.h>

#include "datamodelDcps.h"

typedef struct {

    uint8_t ID;
    uint32_t position;

} balise_t;

bool get_balise_if_linked(const uint8_t ID, balise_t* balise);

bool set_linked_balises(const DDS_sequence_long input_data);

#endif