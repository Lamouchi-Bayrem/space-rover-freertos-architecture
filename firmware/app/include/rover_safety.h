
#ifndef ROVER_SAFETY_H
#define ROVER_SAFETY_H

#include <stdbool.h>
#include <stdint.h>

#include "rover_types.h"

/*
 * ==========================================================================
 * Safety Evaluation
 * ==========================================================================
 *
 * This module contains pure safety decision logic.
 *
 * It does NOT:
 *
 *     - access hardware
 *     - control motors
 *     - access FreeRTOS objects
 *     - modify rover state
 *
 * It only evaluates the supplied inputs and returns fault flags.
 */


/*
 * Evaluate all safety conditions.
 *
 * @param now_ms       Current system time.
 * @param cmd          Latest rover command.
 * @param battery_v    Battery voltage.
 * @param temp_c       Board temperature.
 * @param sensors_ok   Sensor health status.
 * @param link_ok      Communication health status.
 *
 * @return Combined fault bitmask.
 */
uint32_t rover_safety_evaluate(
    uint32_t now_ms,
    const rover_command_t *cmd,
    float battery_v,
    float temp_c,
    bool sensors_ok,
    bool link_ok
);


/*
 * Return true when the current fault set requires
 * the rover to stop.
 *
 * Current policy:
 *
 *     Any safety fault -> STOP
 */
bool rover_safety_requires_stop(
    uint32_t faults
);


/*
 * Check whether a specific fault is active.
 */
bool rover_safety_has_fault(
    uint32_t faults,
    uint32_t fault
);


#endif /* ROVER_SAFETY_H */

