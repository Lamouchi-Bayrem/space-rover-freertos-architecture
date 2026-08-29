#ifndef ROVER_CONFIG_H
#define ROVER_CONFIG_H
#define ROVER_STATE_MUTEX_TIMEOUT_MS    5U
/*
 *==========================================================================*
 *                           TASK STACK SIZES                               *
 *==========================================================================*
 */

#define ROVER_SAFETY_STACK_SIZE       384U
#define ROVER_CONTROL_STACK_SIZE      512U
#define ROVER_IMU_STACK_SIZE          384U
#define ROVER_ENCODER_STACK_SIZE      384U
#define ROVER_COMMS_STACK_SIZE       1024U
#define ROVER_TELEMETRY_STACK_SIZE    512U


/*
 *==========================================================================*
 *                           TASK PRIORITIES                                *
 *==========================================================================*
 */

#define ROVER_SAFETY_TASK_PRIORITY       5U
#define ROVER_CONTROL_TASK_PRIORITY      4U
#define ROVER_IMU_TASK_PRIORITY          3U
#define ROVER_ENCODER_TASK_PRIORITY      3U
#define ROVER_COMMS_TASK_PRIORITY        2U
#define ROVER_TELEMETRY_TASK_PRIORITY    1U


/*
 *==========================================================================*
 *                              PERIODS                                     *
 *==========================================================================*
 */

#define ROVER_SAFETY_PERIOD_MS          20U
#define ROVER_CONTROL_PERIOD_MS         10U
#define ROVER_ENCODER_PERIOD_MS         10U
#define ROVER_COMMS_PERIOD_MS            5U
#define ROVER_TELEMETRY_PERIOD_MS      100U


/*
 *==========================================================================*
 *                            TIMEOUTS                                      *
 *==========================================================================*
 */

/*
 * Maximum allowed time between valid IMU samples.
 */
#define ROVER_IMU_TIMEOUT_MS            100U

/*
 * Maximum allowed time between valid encoder samples.
 */
#define ROVER_ENCODER_TIMEOUT_MS        100U

/*
 * Maximum allowed time since the last valid command.
 *
 * If exceeded, communication is considered lost.
 */
#define ROVER_COMMAND_TIMEOUT_MS        500U


/*
 *==========================================================================*
 *                              QUEUES                                      *
 *==========================================================================*
 */

/*
 * Latest-value queues.
 *
 * Length 1 is intentional because stale sensor data should be
 * overwritten by the newest measurement.
 */
#define ROVER_QUEUE_LENGTH_IMU            1U
#define ROVER_QUEUE_LENGTH_ENCODER        1U

/*
 * Command queue may contain multiple commands.
 */
#define ROVER_COMMAND_QUEUE_LENGTH        8U

#endif /* ROVER_CONFIG_H */
