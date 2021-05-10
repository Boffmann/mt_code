#ifndef __DECISION_MAKING_H__
#define __DECISION_MAKING_H__

#include <stdbool.h>
#include <stdint.h>

#include "datamodelDcps.h"
#include "evaluation/evaluator.h"

// I support a binary decision: Break or continue journey
/**
 * Decides whether the train should brake or continue its journey. Is used both for validating balise telegrams and
 * calculating braking curve
 * 
 * @param is_balise_decision true when a balise telegram should be evaluated, false when it's a sole braking curve calculation
 * @param baliseID ID for the balise whose telegram is evaluated. 0 for no balise
 * @param reason Pointer to a reason object used to specify why the train should brake
 * 
 * @returns true when train should brake, false otherwise
 */
bool decide_should_brake(const bool is_balise_decision, const int baliseID, enum StoppedReason *reason);

#endif