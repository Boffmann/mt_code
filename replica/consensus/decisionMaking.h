#ifndef __DECISION_MAKING_H__
#define __DECISION_MAKING_H__

#include <stdbool.h>
#include <stdint.h>

#define MAX_POSITION_DRIFT 100

// I support a binary decision: Break or continue journey
// Returns true means continue, false means break
bool process_input(const char* data, const uint8_t data_length);

#endif