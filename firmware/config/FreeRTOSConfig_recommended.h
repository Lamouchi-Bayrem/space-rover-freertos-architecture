/*
 * ============================================================================
 * FreeRTOS Configuration Recommendations
 * ============================================================================
 *
 * File:
 *     firmware/config/FreeRTOSConfig_recommended.h
 *
 * Purpose:
 *     Recommended FreeRTOS configuration for the Space Rover firmware.
 *
 * IMPORTANT:
 *     This file is a configuration reference.
 *
 *     Copy the required definitions into the CubeMX-generated
 *     FreeRTOSConfig.h for the STM32 target.
 *
 * Architecture:
 *
 *     - Preemptive scheduler
 *     - 1 ms RTOS tick
 *     - Static memory allocation
 *     - No dynamic allocation
 *     - Mutex synchronization
 *     - Task notifications
 *     - Software timers
 *     - Stack overflow detection
 *     - Runtime stack monitoring
 *
 * ============================================================================
 */

#ifndef FREERTOS_CONFIG_RECOMMENDED_H
#define FREERTOS_CONFIG_RECOMMENDED_H


/* ============================================================================
 * Scheduler Configuration
 * ========================================================================== */

/*
 * Enable preemptive scheduling.
 *
 * Higher-priority tasks can interrupt lower-priority tasks.
 */
#define configUSE_PREEMPTION                    1


/*
 * Allow tasks with the same priority to share CPU time.
 */
#define configUSE_TIME_SLICING                  1


/*
 * Use 32-bit TickType_t.
 *
 * This provides a large timing range for long-running systems.
 */
#define configUSE_16_BIT_TICKS                  0


/*
 * FreeRTOS scheduler tick frequency.
 *
 * 1000 Hz = 1 ms tick resolution.
 *
 * Rover timing:
 *
 *     Communications :   5 ms
 *     Control        :  10 ms
 *     Encoder        :  10 ms
 *     Safety         :  20 ms
 *     Telemetry      : 100 ms
 */
#define configTICK_RATE_HZ                      1000U


/*
 * Maximum number of task priorities.
 *
 * Current rover priorities:
 *
 *     Safety       = 5
 *     Control      = 4
 *     IMU          = 3
 *     Encoder      = 3
 *     Communications = 2
 *     Telemetry    = 1
 */
#define configMAX_PRIORITIES                    8U


/* ============================================================================
 * Memory Management
 * ========================================================================== */

/*
 * Enable static allocation.
 *
 * The rover application uses:
 *
 *     xTaskCreateStatic()
 *     xQueueCreateStatic()
 *     xEventGroupCreateStatic()
 *     xSemaphoreCreateMutexStatic()
 */
#define configSUPPORT_STATIC_ALLOCATION         1


/*
 * Disable dynamic allocation.
 *
 * This prevents runtime heap allocation in the rover application.
 */
#define configSUPPORT_DYNAMIC_ALLOCATION        0


/* ============================================================================
 * Safety and Diagnostics
 * ========================================================================== */

/*
 * Enable FreeRTOS stack overflow checking.
 *
 * Value 2 provides the stronger FreeRTOS stack checking mechanism.
 */
#define configCHECK_FOR_STACK_OVERFLOW          2


/*
 * Enable malloc failure hook.
 *
 * Dynamic allocation is disabled, but keeping the hook enabled provides
 * a defensive failure path if a component unexpectedly requests memory.
 */
#define configUSE_MALLOC_FAILED_HOOK            1


/*
 * Optional FreeRTOS assertion support.
 *
 * Define configASSERT in the actual FreeRTOSConfig.h if you want
 * centralized assertion handling.
 *
 * Example:
 *
 *     #define configASSERT(x) rover_freertos_assert((x))
 *
 * The implementation should be placed in freertos_hooks.c.
 */


/* ============================================================================
 * Synchronization
 * ========================================================================== */

/*
 * Enable mutexes.
 *
 * Used by rover_state.c to protect shared command/status state.
 */
#define configUSE_MUTEXES                       1


/*
 * Recursive mutexes are not required by the current architecture.
 */
#define configUSE_RECURSIVE_MUTEXES             0


/*
 * Counting semaphores are available for future synchronization mechanisms.
 */
#define configUSE_COUNTING_SEMAPHORES            1


/* ============================================================================
 * Task Notifications
 * ========================================================================== */

/*
 * Enable task notifications.
 *
 * IMU interrupt:
 *
 *     vTaskNotifyGiveFromISR()
 *
 * IMU task:
 *
 *     ulTaskNotifyTake()
 *
 * This provides a lightweight ISR-to-task synchronization mechanism.
 */
#define configUSE_TASK_NOTIFICATIONS             1


/* ============================================================================
 * Software Timers
 * ========================================================================== */

/*
 * Enable FreeRTOS software timers.
 */
#define configUSE_TIMERS                         1


/*
 * Timer service task priority.
 *
 * Keep this below the Safety and Control tasks.
 */
#define configTIMER_TASK_PRIORITY                2U


/*
 * Number of timer commands that can be queued.
 */
#define configTIMER_QUEUE_LENGTH                 8U


/*
 * Timer service task stack depth.
 *
 * This value must be validated using stack high-water-mark measurements.
 */
#define configTIMER_TASK_STACK_DEPTH             512U


/* ============================================================================
 * FreeRTOS API Includes
 * ========================================================================== */

/*
 * Required by rover periodic tasks.
 */
#define INCLUDE_vTaskDelay                       1

#define INCLUDE_vTaskDelayUntil                 1


/*
 * Required for stack monitoring.
 *
 * Example:
 *
 *     uxTaskGetStackHighWaterMark()
 */
#define INCLUDE_uxTaskGetStackHighWaterMark      1


/*
 * Optional useful APIs for diagnostics.
 *
 * Enable these if the application needs them.
 */
#define INCLUDE_xTaskGetCurrentTaskHandle       1

#define INCLUDE_xTaskGetSchedulerState          1


#endif /* FREERTOS_CONFIG_RECOMMENDED_H */
