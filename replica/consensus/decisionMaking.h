#ifndef __DECISION_MAKING_H__
#define __DECISION_MAKING_H__

#include <stdbool.h>
#include <stdint.h>

#include "datamodelDcps.h"
#include "evaluation/evaluator.h"

#define MAX_POSITION_DRIFT 100

// I support a binary decision: Break or continue journey
// bool decide_should_brake(const int* data, const uint8_t data_length);
bool decide_should_brake(DDS_sequence_long *data, enum StoppedReason *reason);

#endif