/*
 * ==========================================================================
 * FreeRTOS Configuration Recommendations
 * ==========================================================================
 *
 * This file is NOT the actual FreeRTOSConfig.h.
 *
 * Copy the relevant definitions into the CubeMX-generated
 * FreeRTOSConfig.h for the STM32 target.
 *
 * Target architecture:
 *     STM32 Cortex-M
 *     FreeRTOS
 *     Static memory allocation
 *     Safety-oriented rover application
 */

#ifndef FREERTOS_CONFIG_RECOMMENDED_H
#define FREERTOS_CONFIG_RECOMMENDED_H


/* ==========================================================================
 * Scheduler
 * ========================================================================== */

/*
 * Enable preemptive scheduling.
 *
 * Higher-priority tasks can interrupt lower-priority tasks.
 */
#define configUSE_PREEMPTION                    1


/*
 * Allow tasks of equal priority to share CPU time.
 */
#define configUSE_TIME_SLICING                  1


/*
 * Use 32-bit TickType_t.
 *
 * Important for long-running embedded systems.
 */
#define configUSE_16_BIT_TICKS                  0


/*
 * 1 ms RTOS tick.
 *
 * This gives good timing resolution for:
 *
 *     Safety
 *     Control
 *     Sensor
 *     Communication
 *
 * tasks.
 */
#define configTICK_RATE_HZ                      1000U


/*
 * Maximum number of task priorities.
 *
 * Your rover currently uses priorities:
 *
 *     Safety       = 5
 *     Control      = 4
 *     IMU          = 3
 *     Encoder      = 3
 *     Communications = 2
 *     Telemetry    = 1
 */
#define configMAX_PRIORITIES                    8U


/* ==========================================================================
 * Memory Management
 * ========================================================================== */

/*
 * Static allocation is REQUIRED by the rover architecture.
 *
 * Tasks, queues and event groups are statically allocated.
 */
#define configSUPPORT_STATIC_ALLOCATION         1


/*
 * Dynamic allocation is disabled.
 *
 * This prevents unexpected heap usage at runtime.
 */
#define configSUPPORT_DYNAMIC_ALLOCATION        0


/* ==========================================================================
 * Safety / Diagnostics
 * ========================================================================== */

/*
 * Enable the strongest FreeRTOS stack overflow check.
 */
#define configCHECK_FOR_STACK_OVERFLOW          2


/*
 * Called when a dynamic allocation fails.
 *
 * Although dynamic allocation is disabled for the application,
 * keeping the hook enabled provides an additional safety mechanism
 * if some FreeRTOS component unexpectedly attempts allocation.
 */
#define configUSE_MALLOC_FAILED_HOOK            1


/* ==========================================================================
 * Synchronization
 * ========================================================================== */

/*
 * Required for the rover state mutex.
 */
#define configUSE_MUTEXES                       1


/*
 * Recursive mutexes are not required.
 */
#define configUSE_RECURSIVE_MUTEXES             0


/*
 * Useful for future synchronization between drivers/tasks.
 */
#define configUSE_COUNTING_SEMAPHORES            1


/* ==========================================================================
 * Task Notifications
 * ========================================================================== */

/*
 * Used by the IMU interrupt.
 *
 * ISR:
 *
 *     vTaskNotifyGiveFromISR()
 *
 * Task:
 *
 *     ulTaskNotifyTake()
 */
#define configUSE_TASK_NOTIFICATIONS             1


/* ==========================================================================
 * Software Timers
 * ========================================================================== */

#define configUSE_TIMERS                         1

#define configTIMER_TASK_PRIORITY                2U

#define configTIMER_QUEUE_LENGTH                 8U

#define configTIMER_TASK_STACK_DEPTH             512U


/* ==========================================================================
 * Required API Functions
 * ========================================================================== */

/*
 * Required by the rover periodic tasks.
 */
#define INCLUDE_vTaskDelay                       1

#define INCLUDE_vTaskDelayUntil                 1


/*
 * Required for runtime stack monitoring.
 *
 * Example:
 *
 *     uxTaskGetStackHighWaterMark()
 */
#define INCLUDE_uxTaskGetStackHighWaterMark      1


#endif /* FREERTOS_CONFIG_RECOMMENDED_H */
