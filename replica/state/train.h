#ifndef __REVPI_TRAIN_H__
#define __REVPI_TRAIN_H__

#include <stdint.h>
#include <stdbool.h>

#define TRAIN_SPEED 1                   // Train speed in m/s
#define TRAIN_DELECERATION 0.4          // Train's deceleration abilities in m/(s^2)
#define MAX_POS_INACCURACY 0.1          // Simulated accuracy for the position sensors

/**
 * Structure for a train's position with confidence interval
 */
typedef struct {
    double min_position;                // minimal position for the confidence interval
    double position;                    // Estimated train position
    double max_position;                // maximum position for the confidence interval
} train_position_t;

/**
 * Structure for a train's state
 */
typedef struct {
    train_position_t position;          // The train's position
    float speed;                        // The train's speed
    bool is_driving;                    // Indicator whether the train is driving or not
    unsigned long long lastUpdateTime;  // Last time that the state was updated. Used to simulate a constant speed
} train_state_t;

/**
 * Setup the train's state for the first time
 */
void initialize_train_state();

/**
 * Simulate the train's position based on a constant speed
 * Adds the updated simulated position to global system state
 */
void update_train_position();

/**
 * Set the train's driving status in global state
 * 
 * @param is_driving driving status to set
 */
void set_train_driving(const bool is_driving);

/**
 * Set the train's position in global state
 * 
 * @param position the position to set
 */
void set_train_position(const double position);

/**
 * Getter that reads the train's current position from global system state
 * 
 * @param train_state pointer to a data structure where the train's current state is written to
 * 
 * @returns true when train was read successfully, false otherwise
 */
bool get_train_state(train_state_t* train_state);

#endif