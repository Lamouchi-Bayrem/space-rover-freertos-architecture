#ifndef ROVER_STATE_H
#define ROVER_STATE_H

#include <stdbool.h>
#include <stdint.h>


/*
 *==========================================================================*
 *                           ROVER MODES                                   *
 *==========================================================================*
 */

typedef enum
{
    ROVER_MODE_BOOT = 0,
    ROVER_MODE_SAFE,
    ROVER_MODE_READY,
    ROVER_MODE_ACTIVE,
    ROVER_MODE_FAULT

} rover_mode_t;


/*
 *==========================================================================*
 *                          ROVER COMMAND                                  *
 *==========================================================================*
 */

typedef struct
{
    float linear_mps;
    float angular_rps;

    /*
     * Timestamp of the last valid command.
     *
     * Used by the safety supervisor to detect communication timeout.
     */
    uint32_t received_at_ms;

} rover_command_t;


/*
 *==========================================================================*
 *                           ROVER STATUS                                  *
 *==========================================================================*
 */

typedef struct
{
    rover_mode_t mode;

    float battery_v;
    float board_temp_c;

    uint32_t fault_flags;

    /*
     * Incremented periodically by the safety task.
     *
     * Used for diagnostics/watchdog supervision.
     */
    uint32_t heartbeat;

} rover_status_t;


/*
 *==========================================================================*
 *                          PUBLIC API                                     *
 *==========================================================================*
 */

void rover_state_init(void);

void rover_state_set_command(
    const rover_command_t *command
);

rover_command_t rover_state_get_command(void);

void rover_state_set_status(
    const rover_status_t *status
);

rover_status_t rover_state_get_status(void);

#endif /* ROVER_STATE_H */
