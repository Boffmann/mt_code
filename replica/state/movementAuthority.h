#ifndef __REVPI_MOVEMENT_AUTHORITY_H__
#define __REVPI_MOVEMENT_AUTHORITY_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t end_position;
    uint8_t target_speed;
} movement_authority_t;

void set_movement_authority(const movement_authority_t* ma);
bool get_movement_authority(movement_authority_t* ma);

#endif