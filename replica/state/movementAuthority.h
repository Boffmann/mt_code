#ifndef __REVPI_MOVEMENT_AUTHORITY_H__
#define __REVPI_MOVEMENT_AUTHORITY_H__

#include <stdint.h>
#include <stdbool.h>

#include "datamodelDcps.h"

typedef struct {
    uint32_t start_position;
    uint32_t end_position;
} movement_authority_t;

void set_movement_authority(const movement_authority_t* ma);
bool parse_and_set_movement_authority(const DDS_sequence_long input_data);
bool get_movement_authority(movement_authority_t* ma);

#endif