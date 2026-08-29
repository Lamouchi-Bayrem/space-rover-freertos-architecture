
#ifndef ROVER_CONFIG_H
#define ROVER_CONFIG_H


/*
 * ==========================================================================
 * FreeRTOS Task Configuration
 * ==========================================================================
 */

/* Stack sizes are expressed in FreeRTOS StackType_t units. */

#define ROVER_SAFETY_STACK_SIZE       384U
#define ROVER_CONTROL_STACK_SIZE      512U
#define ROVER_IMU_STACK_SIZE          384U
#define ROVER_ENCODER_STACK_SIZE      384U
#define ROVER_COMMS_STACK_SIZE       1024U
#define ROVER_TELEMETRY_STACK_SIZE    512U


#define ROVER_SAFETY_TASK_PRIORITY       5U
#define ROVER_CONTROL_TASK_PRIORITY      4U
#define ROVER_IMU_TASK_PRIORITY          3U
#define ROVER_ENCODER_TASK_PRIORITY      3U
#define ROVER_COMMS_TASK_PRIORITY        2U
#define ROVER_TELEMETRY_TASK_PRIORITY    1U


/*
 * ==========================================================================
 * Task Periods
 * ==========================================================================
 */

#define ROVER_SAFETY_PERIOD_MS          20U
#define ROVER_CONTROL_PERIOD_MS         10U
#define ROVER_ENCODER_PERIOD_MS         10U
#define ROVER_COMMS_PERIOD_MS            5U
#define ROVER_TELEMETRY_PERIOD_MS      100U


/*
 * ==========================================================================
 * Safety Timeouts
 * ==========================================================================
 */

/* Maximum allowed age of an IMU measurement. */
#define ROVER_IMU_TIMEOUT_MS            100U

/* Maximum allowed age of an encoder measurement. */
#define ROVER_ENCODER_TIMEOUT_MS        100U

/* Maximum allowed age of a valid command. */
#define ROVER_COMMAND_TIMEOUT_MS        500U


/*
 * ==========================================================================
 * Safety Thresholds
 * ==========================================================================
 */

/*
 * Minimum allowed battery voltage.
 *
 * Replace this value with the actual value for your battery system.
 */
#define ROVER_LOW_BATTERY_V             10.5F


/*
 * Maximum allowed board temperature.
 *
 * Replace this value with the actual hardware limit.
 */
#define ROVER_MAX_BOARD_TEMP_C          80.0F


/*
 * ==========================================================================
 * State Manager
 * ==========================================================================
 */

/*
 * Maximum time the state manager waits for its mutex.
 */
#define ROVER_STATE_MUTEX_TIMEOUT_MS      5U


/*
 * ==========================================================================
 * Queue Configuration
 * ==========================================================================
 */

/*
 * Single-element queues are intentional.
 *
 * They represent "latest sample" buffers.
 */
#define ROVER_QUEUE_LENGTH_IMU            1U
#define ROVER_QUEUE_LENGTH_ENCODER        1U


/*
 * Commands can temporarily accumulate.
 */
#define ROVER_COMMAND_QUEUE_LENGTH        8U


#endif /* ROVER_CONFIG_H */

