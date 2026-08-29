#ifndef ROVER_SAFETY_H
#define ROVER_SAFETY_H

#include <stdbool.h>
#include <stdint.h>

#include "rover_state.h"

/*
 *==========================================================================*
 *                           FAULT FLAGS                                    *
 *==========================================================================*
 *
 * Each fault occupies one bit.
 *
 * This allows multiple faults to exist simultaneously.
 *
 * Example:
 *
 *     LOW_BATTERY | SENSOR
 *
 * becomes:
 *
 *     00000001
 *   | 00000100
 *     --------
 *     00000101
 */

#define ROVER_FAULT_NONE              (0UL)
#define ROVER_FAULT_COMMAND_TIMEOUT   (1UL << 0U)
#define ROVER_FAULT_LOW_BATTERY       (1UL << 1U)
#define ROVER_FAULT_OVERTEMP          (1UL << 2U)
#define ROVER_FAULT_SENSOR            (1UL << 3U)
#define ROVER_FAULT_LINK              (1UL << 4U)


/*
 * Evaluate all safety conditions.
 *
 * Returns a bitmask containing all currently detected faults.
 */
uint32_t rover_safety_evaluate(
    uint32_t now,
    const rover_command_t *cmd,
    float battery,
    float temp,
    bool sensors_ok,
    bool link_ok
);


/*
 * Determine whether the current fault set requires
 * the motors to stop.
 */
bool rover_safety_requires_stop(uint32_t faults);


/*
 * Optional helper functions.
 *
 * These make the safety logic easier to test and understand.
 */
bool rover_safety_has_fault(uint32_t faults, uint32_t fault);

#endif /* ROVER_SAFETY_H */
