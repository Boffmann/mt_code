#ifndef __REVPI_TRAIN_H__
#define __REVPI_TRAIN_H__

#include <stdint.h>
#include <stdbool.h>

#define TRAIN_SPEED 1  // Train speed in m/s
#define TRAIN_DELECERATION 0.4
#define MAX_POS_INACCURACY 0.4

typedef struct {
    double min_position;
    double position;
    double max_position;
} train_position_t;

typedef struct {
    train_position_t position;
    float speed;
    unsigned long long lastUpdateTime;
} train_state_t;

// void initialize_train_state();
void update_train_state();

void set_train_position(const double position);

bool get_train_state(train_state_t* train_state);

#endif