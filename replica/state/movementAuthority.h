#ifndef __REVPI_MOVEMENT_AUTHORITY_H__
#define __REVPI_MOVEMENT_AUTHORITY_H__

#include <stdint.h>
#include <stdbool.h>

#include "datamodelDcps.h"

/**
 * Structure for a movement authority
 */
typedef struct {
    uint32_t start_position;    // The movement authorities' start position
    uint32_t end_position;      // The movement authorities' end position 
} movement_authority_t;

/**
 * Setter to publish a new movement authority to the global system state
 */
void set_movement_authority(const movement_authority_t* ma);

/**
 * Setter to parse and publish a new movement authority to the global system state
 * 
 * @param input_data Input data to parse movement authority from
 * 
 * @returns true if parsing and adding movement authority was successful, false otherwise
 */
bool parse_and_set_movement_authority(const DDS_sequence_long input_data);

/**
 * Getter for movement authority
 * The information is taken from the global system state
 * 
 * @param ma Pointer to a structure where the ma information should be written to
 * 
 * @returns true if movement authority has been written successfully, false otherwise
 */
bool get_movement_authority(movement_authority_t* ma);

#endif