#include "rover_safety.h"
#include "rover_config.h"


/*
 *==========================================================================*
 *                         SAFETY EVALUATION                                *
 *==========================================================================*
 */

/**
 * @brief Evaluate all rover safety conditions.
 *
 * @param now          Current system time in milliseconds.
 * @param cmd          Latest rover command.
 * @param battery      Battery voltage.
 * @param temp         Board temperature.
 * @param sensors_ok   Sensor health status.
 * @param link_ok      Communication link status.
 *
 * @return Bitmask containing all detected faults.
 *
 * The function does not change system state and does not control
 * hardware. It only evaluates conditions.
 *
 * This makes the function deterministic and easy to unit-test.
 */
uint32_t rover_safety_evaluate(
    uint32_t now,
    const rover_command_t *cmd,
    float battery,
    float temp,
    bool sensors_ok,
    bool link_ok)
{
    uint32_t faults = ROVER_FAULT_NONE;


    /*
     *------------------------------------------------------------------*
     * Command timeout
     *------------------------------------------------------------------*
     *
     * A NULL command is considered unsafe.
     *
     * The subtraction:
     *
     *     now - received_at_ms
     *
     * is intentionally performed using unsigned arithmetic.
     *
     * This allows the calculation to remain valid across uint32_t
     * timer wraparound.
     */
    if (cmd == NULL)
    {
        faults |= ROVER_FAULT_COMMAND_TIMEOUT;
    }
    else if ((uint32_t)(now - cmd->received_at_ms) >
             ROVER_COMMAND_TIMEOUT_MS)
    {
        faults |= ROVER_FAULT_COMMAND_TIMEOUT;
    }


    /*
     *------------------------------------------------------------------*
     * Battery monitoring
     *------------------------------------------------------------------*
     */
    if (battery < ROVER_LOW_BATTERY_V)
    {
        faults |= ROVER_FAULT_LOW_BATTERY;
    }


    /*
     *------------------------------------------------------------------*
     * Temperature monitoring
     *------------------------------------------------------------------*
     */
    if (temp > ROVER_MAX_BOARD_TEMP_C)
    {
        faults |= ROVER_FAULT_OVERTEMP;
    }


    /*
     *------------------------------------------------------------------*
     * Sensor health
     *------------------------------------------------------------------*
     */
    if (!sensors_ok)
    {
        faults |= ROVER_FAULT_SENSOR;
    }


    /*
     *------------------------------------------------------------------*
     * Communication health
     *------------------------------------------------------------------*
     */
    if (!link_ok)
    {
        faults |= ROVER_FAULT_LINK;
    }


    return faults;
}


/*
 *==========================================================================*
 *                       SAFETY STOP DECISION                               *
 *==========================================================================*
 */

/**
 * @brief Determine whether the rover must stop.
 *
 * Current policy:
 *
 *     Any detected safety fault -> STOP
 *
 * This is intentionally conservative.
 *
 * Later, the project can distinguish between:
 *
 *     WARNING
 *     DEGRADED
 *     CRITICAL
 *
 * and only critical faults would force an immediate motor shutdown.
 */
bool rover_safety_requires_stop(uint32_t faults)
{
    return (faults != ROVER_FAULT_NONE);
}


/*
 *==========================================================================*
 *                          FAULT HELPER                                    *
 *==========================================================================*
 */

bool rover_safety_has_fault(
    uint32_t faults,
    uint32_t fault)
{
    return ((faults & fault) != 0UL);
}
