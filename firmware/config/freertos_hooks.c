/*
 * ============================================================================
 * FreeRTOS Failure Hooks
 * ============================================================================
 *
 * File:
 *     firmware/config/freertos_hooks.c
 *
 * Purpose:
 *     Handle critical FreeRTOS failures.
 *
 * Failure conditions handled here:
 *
 *     1. Task stack overflow
 *     2. Memory allocation failure
 *     3. FreeRTOS assertion failure
 *
 * Safety philosophy:
 *
 *     Critical RTOS failure
 *             |
 *             v
 *       Emergency stop
 *             |
 *             v
 *      Disable interrupts
 *             |
 *             v
 *        Fail-safe loop
 *
 * IMPORTANT:
 *
 *     Failure hooks must not perform blocking operations.
 *
 *     Do NOT:
 *
 *         - call printf()
 *         - allocate memory
 *         - wait on a mutex
 *         - wait on a semaphore
 *         - call vTaskDelay()
 *         - depend on another task
 *
 * ============================================================================
 */

#include "FreeRTOS.h"
#include "task.h"

#include "rover_hal.h"


/* ============================================================================
 * Private Failure Handler
 * ========================================================================== */

/**
 * @brief Enter the rover fail-safe state.
 *
 * This function is used when the FreeRTOS kernel detects a critical
 * configuration/runtime failure.
 *
 * The emergency stop must be implemented at the HAL level without
 * depending on the FreeRTOS scheduler.
 */
static void rover_freertos_fail_safe(void)
{
    /*
     * Force the motor outputs into their safe state.
     *
     * IMPORTANT:
     *
     * rover_hal_emergency_stop() must NOT use:
     *
     *     mutexes
     *     queues
     *     task notifications
     *     delays
     *     dynamic memory
     *
     * It should directly force the motor control hardware to a
     * non-driving state.
     */
    rover_hal_emergency_stop();


    /*
     * Stop normal interrupt processing.
     *
     * The firmware is no longer considered operational.
     */
    taskDISABLE_INTERRUPTS();


    /*
     * Remain permanently in the fail-safe state.
     *
     * A watchdog can be used to reset the MCU if desired.
     */
    for (;;)
    {
        /*
         * Intentionally empty.
         *
         * Possible future diagnostic action:
         *
         *     - fault LED
         *     - retained fault code
         *     - watchdog reset
         */
    }
}


/* ============================================================================
 * Stack Overflow Hook
 * ========================================================================== */

/**
 * @brief FreeRTOS stack overflow callback.
 *
 * Called by FreeRTOS when a task stack overflow is detected.
 *
 * @param task Task handle associated with the failure.
 * @param name Task name.
 */
void vApplicationStackOverflowHook(
    TaskHandle_t task,
    char *name)
{
    /*
     * The parameters are intentionally unused for now.
     *
     * They can later be stored in a diagnostic structure before
     * entering the fail-safe state.
     */
    (void)task;
    (void)name;


    /*
     * A stack overflow is considered a critical firmware failure.
     */
    rover_freertos_fail_safe();
}


/* ============================================================================
 * Malloc Failure Hook
 * ========================================================================== */

/**
 * @brief FreeRTOS allocation failure callback.
 *
 * The rover application disables dynamic allocation.
 *
 * Therefore reaching this function indicates an unexpected configuration
 * or library/component attempting to allocate memory dynamically.
 */
void vApplicationMallocFailedHook(void)
{
    rover_freertos_fail_safe();
}


/* ============================================================================
 * FreeRTOS Assertion Handler
 * ========================================================================== */

/**
 * @brief Application-level FreeRTOS assertion handler.
 *
 * This function can be connected to configASSERT().
 *
 * Example in FreeRTOSConfig.h:
 *
 *     #define configASSERT(x) rover_freertos_assert((x))
 *
 * @param condition Assertion condition.
 */
void rover_freertos_assert(
    BaseType_t condition)
{
    /*
     * Nothing happens when the assertion is valid.
     */
    if (condition != pdFALSE)
    {
        return;
    }


    /*
     * Invalid assertion:
     *
     * Enter the rover fail-safe state.
     */
    rover_freertos_fail_safe();
}
