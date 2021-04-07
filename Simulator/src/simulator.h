#ifndef __REVPI_SIMULATOR_H__
#define __REVPI_SIMULATOR_H__

#include "dds_dcps.h"
#include <stdint.h>

#include "scenario.h"

extern DDS_DomainParticipant domainParticipant;
extern uint32_t inputID;

void simulator_init();

void send_linking_information(balise_array_t* linked_balises);

void send_movement_authority(scenario_t* scenario);

void send_balise(const balise_t* const balise);
void send_terminate();

#endif