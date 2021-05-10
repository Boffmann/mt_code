#ifndef __REVPI_BALISE_H__
#define __REVPI_BALISE_H__

#include <stdint.h>
#include <stdbool.h>

#include "datamodelDcps.h"

/**
 * Structure of a balise that is linked to the unit
 */
typedef struct {

    uint8_t ID;             // An unique idenfifier for a balise
    uint32_t position;      // The position at which the balise is linked

} balise_t;

/**
 * Getter for a balise when it is linked.
 * The information is taken from the global system state
 * 
 * @param ID Id of the balise that should be taken from global state
 * @param balise Pointer to a balise_t object where the information for the linked balise should be written to
 * 
 * @returns true when balise with ID is linked, false otherwise
 */
bool get_balise_if_linked(const uint8_t ID, balise_t* balise);

/**
 * Setter to parse and add new linked balises into the global system state
 * 
 * @param input_data Linking data is received as a stream of data
 * 
 * @returns true if parsing and publishing linked balise was successfull, false otherwise
 */
bool set_linked_balises(const DDS_sequence_long input_data);

#endif