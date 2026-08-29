
#ifndef ROVER_STATE_H
#define ROVER_STATE_H

#include "rover_types.h"

/*
 * ==========================================================================
 * Rover State Manager
 * ==========================================================================
 *
 * This module owns the shared application state.
 *
 * FreeRTOS tasks must access the state through these functions rather
 * than accessing the internal variables directly.
 */


/*
 * Initialize the rover state manager.
 *
 * Initial state:
 *
 *     Mode       = BOOT
 *     Command    = zero
 *     Faults     = NONE
 *     Heartbeat  = 0
 */
void rover_state_init(void);


/*
 * ==========================================================================
 * Command API
 * ==========================================================================
 */

void rover_state_set_command(
    const rover_command_t *command
);

rover_command_t rover_state_get_command(void);


/*
 * ==========================================================================
 * Status API
 * ==========================================================================
 */

void rover_state_set_status(
    const rover_status_t *status
);

rover_status_t rover_state_get_status(void);


#endif /* ROVER_STATE_H */

