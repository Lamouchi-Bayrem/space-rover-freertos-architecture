/*
 * rover_tasks.c
 *
 * FreeRTOS application tasks for the rover.
 *
 * Architecture:
 *
 *              +----------------------+
 *              |   Safety Supervisor  |
 *              +----------+-----------+
 *                         |
 *                         v
 *              +----------------------+
 *              |    Rover State       |
 *              +----------+-----------+
 *                         ^
 *                         |
 *       +-----------------+-----------------+
 *       |                 |                 |
 *       v                 v                 v
 *   IMU Task        Encoder Task       Comms Task
 *       |                 |                 |
 *       +--------+--------+                 |
 *                v                          |
 *         State Estimator <-----------------+
 *                |
 *                v
 *         Control Task
 *                |
 *                v
 *            Motor HAL
 *
 * Important design rules:
 *
 * 1. Tasks use static memory allocation.
 * 2. ISRs perform only minimal work.
 * 3. Sensor freshness is monitored using timestamps.
 * 4. Communication health is based on activity/timeout,
 *    not simply on the communication task executing.
 * 5. Safety has the highest application priority.
 * 6. Queue/API return values are checked.
 *
 * Compile this file only for the STM32/FreeRTOS target.
 */

#include "rover_tasks.h"
#include "rover_config.h"
#include "rover_safety.h"
#include "rover_state.h"
#include "rover_hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include <stdint.h>
#include <stdbool.h>


/*==========================================================================*/
/*                           EVENT DEFINITIONS                              */
/*==========================================================================*/

/*
 * Event groups allow multiple tasks to publish system-health information.
 *
 * EVENT_LINK_OK:
 *     Communication link is considered healthy.
 *
 * EVENT_SENSOR_OK:
 *     Required sensors are considered healthy and fresh.
 */
#define ROVER_EVENT_LINK_OK       (1UL << 0U)
#define ROVER_EVENT_SENSOR_OK     (1UL << 1U)


/*==========================================================================*/
/*                         DIAGNOSTIC DEFINITIONS                            */
/*==========================================================================*/

/*
 * These counters provide lightweight runtime diagnostics.
 *
 * They can later be exported through telemetry or a debugger.
 */
typedef struct
{
    uint32_t command_queue_overflows;
    uint32_t imu_read_errors;
    uint32_t encoder_read_errors;
    uint32_t imu_timeouts;
    uint32_t encoder_timeouts;
    uint32_t communication_timeouts;
    uint32_t safety_stops;
} rover_task_diagnostics_t;


/*==========================================================================*/
/*                         STATIC FREERTOS OBJECTS                           */
/*==========================================================================*/

/*
 * Static task control blocks.
 *
 * Static allocation avoids runtime calls to pvPortMalloc().
 * This improves memory determinism and simplifies failure analysis.
 */
static StaticTask_t safety_tcb;
static StaticTask_t control_tcb;
static StaticTask_t imu_tcb;
static StaticTask_t encoder_tcb;
static StaticTask_t comms_tcb;
static StaticTask_t telemetry_tcb;


/*
 * Task stacks.
 *
 * Stack sizes should eventually be verified using
 * uxTaskGetStackHighWaterMark().
 */
static StackType_t safety_stack[ROVER_SAFETY_STACK_SIZE];
static StackType_t control_stack[ROVER_CONTROL_STACK_SIZE];
static StackType_t imu_stack[ROVER_IMU_STACK_SIZE];
static StackType_t encoder_stack[ROVER_ENCODER_STACK_SIZE];
static StackType_t comms_stack[ROVER_COMMS_STACK_SIZE];
static StackType_t telemetry_stack[ROVER_TELEMETRY_STACK_SIZE];


/*
 * Static queue control blocks.
 */
static StaticQueue_t imu_queue_cb;
static StaticQueue_t encoder_queue_cb;
static StaticQueue_t command_queue_cb;


/*
 * Queue storage.
 *
 * The IMU and encoder queues contain one element intentionally.
 *
 * They act as "latest value" buffers:
 *
 *     old sample
 *          |
 *          v
 *     +----------+
 *     | latest   |
 *     | sample   |
 *     +----------+
 *          ^
 *          |
 *      new sample
 *
 * xQueueOverwrite() is valid for a queue of length 1.
 */
static uint8_t imu_queue_storage[
    ROVER_QUEUE_LENGTH_IMU * sizeof(rover_imu_t)
];

static uint8_t encoder_queue_storage[
    ROVER_QUEUE_LENGTH_ENCODER * sizeof(rover_encoder_t)
];

static uint8_t command_queue_storage[
    ROVER_COMMAND_QUEUE_LENGTH * sizeof(rover_command_t)
];


/*
 * Queue handles.
 */
static QueueHandle_t imu_queue;
static QueueHandle_t encoder_queue;
static QueueHandle_t command_queue;


/*
 * Static event group.
 */
static StaticEventGroup_t system_events_cb;
static EventGroupHandle_t system_events;


/*==========================================================================*/
/*                            TASK HANDLES                                   */
/*==========================================================================*/

/*
 * Keeping task handles allows:
 *
 * - task notifications
 * - diagnostics
 * - stack monitoring
 * - future watchdog supervision
 */
static TaskHandle_t safety_task_handle;
static TaskHandle_t control_task_handle;
static TaskHandle_t imu_task_handle;
static TaskHandle_t encoder_task_handle;
static TaskHandle_t comms_task_handle;
static TaskHandle_t telemetry_task_handle;


/*==========================================================================*/
/*                         RUNTIME DIAGNOSTICS                               */
/*==========================================================================*/

static rover_task_diagnostics_t diagnostics;


/*==========================================================================*/
/*                         SENSOR TIMESTAMPS                                 */
/*==========================================================================*/

/*
 * These timestamps are used to determine whether sensor data is fresh.
 *
 * This is important because a sensor may stop generating interrupts.
 *
 * Checking only the last successful read is not enough.
 */
static volatile TickType_t last_imu_update;
static volatile TickType_t last_encoder_update;
static volatile TickType_t last_command_update;


/*==========================================================================*/
/*                         INTERNAL FUNCTIONS                                */
/*==========================================================================*/

static void rover_update_sensor_health(void);
static void rover_update_link_health(void);
static void rover_monitor_task_stacks(void);
static void rover_process_commands(void);


/*==========================================================================*/
/*                            SAFETY TASK                                    */
/*==========================================================================*/

/*
 * Safety task
 *
 * Priority:
 *     Highest application priority.
 *
 * Responsibilities:
 *
 * - Read battery voltage.
 * - Read board temperature.
 * - Evaluate communication health.
 * - Evaluate sensor health.
 * - Evaluate rover command validity.
 * - Force SAFE mode when a critical fault occurs.
 * - Maintain a software heartbeat.
 *
 * The safety task must remain simple and deterministic.
 */
static void safety_task(void *arg)
{
    TickType_t wake_time;
    rover_status_t status;
    rover_command_t command;

    (void)arg;

    wake_time = xTaskGetTickCount();

    for (;;)
    {
        /*
         * Obtain current system state.
         */
        status = rover_state_get_status();
        command = rover_state_get_command();

        /*
         * Update sensor/link health before evaluating safety.
         */
        rover_update_sensor_health();
        rover_update_link_health();

        /*
         * Read physical safety parameters.
         */
        status.battery_v = rover_hal_battery_voltage();
        status.board_temp_c = rover_hal_board_temperature();

        /*
         * Read current event-group state.
         */
        {
            EventBits_t events;

            events = xEventGroupGetBits(system_events);

            /*
             * Evaluate all safety conditions.
             */
            status.fault_flags =
                rover_safety_evaluate(
                    rover_hal_millis(),
                    &command,
                    status.battery_v,
                    status.board_temp_c,
                    ((events & ROVER_EVENT_SENSOR_OK) != 0U),
                    ((events & ROVER_EVENT_LINK_OK) != 0U)
                );
        }

        /*
         * Critical fault -> force SAFE mode.
         *
         * Motor shutdown is performed immediately from the
         * safety task rather than waiting for the control task.
         */
        if (rover_safety_requires_stop(status.fault_flags))
        {
            status.mode = ROVER_MODE_SAFE;

            rover_hal_motor_stop();

            diagnostics.safety_stops++;
        }

        /*
         * Heartbeat is useful for supervision/watchdog logic.
         */
        status.heartbeat++;

        /*
         * Publish updated status.
         */
        rover_state_set_status(&status);

        /*
         * Maintain a precise periodic execution rate.
         */
        vTaskDelayUntil(
            &wake_time,
            pdMS_TO_TICKS(ROVER_SAFETY_PERIOD_MS)
        );
    }
}


/*==========================================================================*/
/*                           CONTROL TASK                                   */
/*==========================================================================*/

/*
 * Control task
 *
 * Responsibilities:
 *
 * - Receive new commands.
 * - Obtain the current rover state.
 * - Verify that the rover is allowed to move.
 * - Generate motor commands.
 *
 * Safety has priority over control.
 *
 * The control task must NEVER command motors when the rover is in
 * SAFE mode or when a safety fault is active.
 */
static void control_task(void *arg)
{
    TickType_t wake_time;
    rover_command_t command;
    rover_status_t status;

    (void)arg;

    wake_time = xTaskGetTickCount();

    /*
     * Start with a neutral command.
     *
     * This prevents accidental motor movement during startup.
     */
    command.linear_mps = 0.0F;
    command.angular_rps = 0.0F;

    for (;;)
    {
        /*
         * Process every command currently waiting in the queue.
         *
         * This prevents old commands from accumulating unnecessarily.
         */
        rover_process_commands();

        /*
         * Read the latest rover status.
         */
        status = rover_state_get_status();

        /*
         * Copy the current command after queue processing.
         */
        command = rover_state_get_command();

        /*
         * Motor command is allowed only when:
         *
         * 1. Rover is ACTIVE.
         * 2. No safety fault requires stopping.
         */
        if ((status.mode == ROVER_MODE_ACTIVE) &&
            (!rover_safety_requires_stop(status.fault_flags)))
        {
            rover_hal_motor_command(
                command.linear_mps,
                command.angular_rps
            );
        }
        else
        {
            /*
             * Fail-safe behavior.
             */
            rover_hal_motor_stop();
        }

        /*
         * Periodic control loop.
         */
        vTaskDelayUntil(
            &wake_time,
            pdMS_TO_TICKS(ROVER_CONTROL_PERIOD_MS)
        );
    }
}


/*==========================================================================*/
/*                              IMU TASK                                    */
/*==========================================================================*/

/*
 * IMU task
 *
 * The IMU interrupt wakes this task using a direct-to-task notification.
 *
 * ISR:
 *
 *     IMU interrupt
 *          |
 *          v
 *     vTaskNotifyGiveFromISR()
 *          |
 *          v
 *     IMU task
 *          |
 *          v
 *     rover_hal_imu_read()
 *
 * The ISR never performs an I2C/SPI transaction.
 */
static void imu_task(void *arg)
{
    rover_imu_t sample;

    (void)arg;

    for (;;)
    {
        /*
         * Sleep until the IMU signals data availability.
         *
         * portMAX_DELAY means this task consumes no CPU while idle.
         */
        ulTaskNotifyTake(
            pdTRUE,
            portMAX_DELAY
        );

        /*
         * Read the IMU sample.
         */
        if (rover_hal_imu_read(&sample))
        {
            /*
             * Queue length is one, so the newest sample replaces
             * the previous sample.
             */
            if (xQueueOverwrite(
                    imu_queue,
                    &sample) != pdPASS)
            {
                /*
                 * This should normally never happen with a length-1
                 * overwrite queue, but the result is still checked.
                 */
            }

            /*
             * Record successful update time.
             */
            last_imu_update = xTaskGetTickCount();

            /*
             * Sensor data is currently valid.
             */
            xEventGroupSetBits(
                system_events,
                ROVER_EVENT_SENSOR_OK
            );
        }
        else
        {
            /*
             * The sensor transaction failed.
             */
            diagnostics.imu_read_errors++;
        }
    }
}


/*==========================================================================*/
/*                           ENCODER TASK                                   */
/*==========================================================================*/

/*
 * Encoder task
 *
 * Reads wheel encoder information periodically.
 *
 * Like the IMU queue, the encoder queue contains only the latest
 * measurement.
 */
static void encoder_task(void *arg)
{
    TickType_t wake_time;
    rover_encoder_t sample;

    (void)arg;

    wake_time = xTaskGetTickCount();

    for (;;)
    {
        if (rover_hal_encoder_read(&sample))
        {
            if (xQueueOverwrite(
                    encoder_queue,
                    &sample) != pdPASS)
            {
                /*
                 * Queue overwrite should normally succeed because
                 * the queue has a single element.
                 */
            }

            /*
             * Record successful encoder update.
             */
            last_encoder_update = xTaskGetTickCount();
        }
        else
        {
            diagnostics.encoder_read_errors++;
        }

        vTaskDelayUntil(
            &wake_time,
            pdMS_TO_TICKS(ROVER_ENCODER_PERIOD_MS)
        );
    }
}


/*==========================================================================*/
/*                         COMMUNICATION TASK                                */
/*==========================================================================*/

/*
 * Communication task
 *
 * This task is reserved for micro-ROS communication.
 *
 * IMPORTANT:
 *
 * The communication task must NOT simply set EVENT_LINK_OK every time
 * it executes.
 *
 * "Task is alive" does NOT mean "communication link is healthy".
 *
 * The link should be considered healthy only when actual communication
 * activity has been detected.
 */
static void communications_task(void *arg)
{
    (void)arg;

    for (;;)
    {
        /*
         * TODO:
         *
         * 1. Execute micro-ROS executor.
         * 2. Process incoming commands.
         * 3. Publish telemetry.
         * 4. Update last_command_update when a valid command
         *    is received.
         *
         * Example:
         *
         *     if (rover_microros_command_received(&command))
         *     {
         *         if (xQueueSend(command_queue, &command, 0)
         *             == pdPASS)
         *         {
         *             last_command_update = xTaskGetTickCount();
         *         }
         *         else
         *         {
         *             diagnostics.command_queue_overflows++;
         *         }
         *     }
         */

        /*
         * Do NOT set ROVER_EVENT_LINK_OK here.
         *
         * rover_update_link_health() determines link status from
         * actual communication activity.
         */

        vTaskDelay(
            pdMS_TO_TICKS(ROVER_COMMS_PERIOD_MS)
        );
    }
}


/*==========================================================================*/
/*                          TELEMETRY TASK                                  */
/*==========================================================================*/

/*
 * Telemetry task
 *
 * Low-priority task used for:
 *
 * - Debug output
 * - System health
 * - Diagnostics
 * - Future telemetry publication
 *
 * Telemetry should never prevent the safety/control tasks from running.
 */
static void telemetry_task(void *arg)
{
    TickType_t wake_time;
    rover_status_t status;

    (void)arg;

    wake_time = xTaskGetTickCount();

    for (;;)
    {
        status = rover_state_get_status();

        /*
         * Existing debug interface.
         */
        rover_hal_debug_status(&status);

        /*
         * Monitor task stack usage.
         */
        rover_monitor_task_stacks();

        vTaskDelayUntil(
            &wake_time,
            pdMS_TO_TICKS(ROVER_TELEMETRY_PERIOD_MS)
        );
    }
}


/*==========================================================================*/
/*                     SENSOR HEALTH MONITOR                                */
/*==========================================================================*/

/*
 * Determine whether sensor data is still fresh.
 *
 * This protects against a situation where:
 *
 *     IMU interrupt stops
 *
 * In that case the IMU task never wakes, so simply checking the last
 * successful read is insufficient.
 */
static void rover_update_sensor_health(void)
{
    TickType_t now;
    TickType_t imu_age;
    TickType_t encoder_age;

    now = xTaskGetTickCount();

    imu_age = now - last_imu_update;
    encoder_age = now - last_encoder_update;

    if ((imu_age <= pdMS_TO_TICKS(ROVER_IMU_TIMEOUT_MS)) &&
        (encoder_age <= pdMS_TO_TICKS(ROVER_ENCODER_TIMEOUT_MS)))
    {
        xEventGroupSetBits(
            system_events,
            ROVER_EVENT_SENSOR_OK
        );
    }
    else
    {
        xEventGroupClearBits(
            system_events,
            ROVER_EVENT_SENSOR_OK
        );

        if (imu_age > pdMS_TO_TICKS(ROVER_IMU_TIMEOUT_MS))
        {
            diagnostics.imu_timeouts++;
        }

        if (encoder_age > pdMS_TO_TICKS(ROVER_ENCODER_TIMEOUT_MS))
        {
            diagnostics.encoder_timeouts++;
        }
    }
}


/*==========================================================================*/
/*                      COMMUNICATION HEALTH                                */
/*==========================================================================*/

/*
 * Communication health is based on the age of the last valid command.
 *
 * Example:
 *
 *     command received
 *           |
 *           v
 *     last_command_update
 *           |
 *           v
 *     timeout timer
 *           |
 *      +----+----+
 *      |         |
 *     OK       timeout
 *                |
 *                v
 *           LINK LOST
 */
static void rover_update_link_health(void)
{
    TickType_t now;
    TickType_t command_age;

    now = xTaskGetTickCount();

    command_age = now - last_command_update;

    if (command_age <=
        pdMS_TO_TICKS(ROVER_COMMAND_TIMEOUT_MS))
    {
        xEventGroupSetBits(
            system_events,
            ROVER_EVENT_LINK_OK
        );
    }
    else
    {
        xEventGroupClearBits(
            system_events,
            ROVER_EVENT_LINK_OK
        );

        diagnostics.communication_timeouts++;
    }
}


/*==========================================================================*/
/*                       COMMAND PROCESSING                                 */
/*==========================================================================*/

/*
 * Drain all pending commands from the command queue.
 *
 * The newest command becomes the active command.
 *
 * This is preferable to processing a backlog of old motion commands.
 */
static void rover_process_commands(void)
{
    rover_command_t command;

    while (xQueueReceive(
        command_queue,
        &command,
        0U) == pdPASS)
    {
        /*
         * Update command freshness.
         */
        last_command_update = xTaskGetTickCount();

        /*
         * Store the newest valid command.
         */
        rover_state_set_command(&command);
    }
}


/*==========================================================================*/
/*                        STACK MONITORING                                  */
/*==========================================================================*/

/*
 * Monitor FreeRTOS stack high-water marks.
 *
 * The returned value is the minimum amount of stack that has remained
 * available during task execution.
 *
 * Lower value = closer to stack exhaustion.
 */
static void rover_monitor_task_stacks(void)
{
    UBaseType_t safety_free;
    UBaseType_t control_free;
    UBaseType_t imu_free;
    UBaseType_t encoder_free;
    UBaseType_t comms_free;
    UBaseType_t telemetry_free;

    safety_free = uxTaskGetStackHighWaterMark(
        safety_task_handle
    );

    control_free = uxTaskGetStackHighWaterMark(
        control_task_handle
    );

    imu_free = uxTaskGetStackHighWaterMark(
        imu_task_handle
    );

    encoder_free = uxTaskGetStackHighWaterMark(
        encoder_task_handle
    );

    comms_free = uxTaskGetStackHighWaterMark(
        comms_task_handle
    );

    telemetry_free = uxTaskGetStackHighWaterMark(
        telemetry_task_handle
    );

    /*
     * Send values to the HAL diagnostic layer.
     *
     * You can implement this later:
     *
     * rover_hal_debug_stack_usage(
     *     safety_free,
     *     control_free,
     *     imu_free,
     *     encoder_free,
     *     comms_free,
     *     telemetry_free
     * );
     */

    (void)safety_free;
    (void)control_free;
    (void)imu_free;
    (void)encoder_free;
    (void)comms_free;
    (void)telemetry_free;
}


/*==========================================================================*/
/*                         APPLICATION INIT                                 */
/*==========================================================================*/

/*
 * Initialize application state and all static FreeRTOS objects.
 *
 * IMPORTANT:
 *
 * Interrupts that depend on task handles should NOT be enabled before
 * rover_create_tasks() has completed.
 */
void rover_app_init(void)
{
    /*
     * Initialize application state.
     */
    rover_state_init();

    /*
     * Initialize hardware abstraction layer.
     */
    rover_hal_init();

    /*
     * Create IMU latest-value queue.
     */
    imu_queue = xQueueCreateStatic(
        ROVER_QUEUE_LENGTH_IMU,
        sizeof(rover_imu_t),
        imu_queue_storage,
        &imu_queue_cb
    );

    /*
     * Create encoder latest-value queue.
     */
    encoder_queue = xQueueCreateStatic(
        ROVER_QUEUE_LENGTH_ENCODER,
        sizeof(rover_encoder_t),
        encoder_queue_storage,
        &encoder_queue_cb
    );

    /*
     * Create command queue.
     */
    command_queue = xQueueCreateStatic(
        ROVER_COMMAND_QUEUE_LENGTH,
        sizeof(rover_command_t),
        command_queue_storage,
        &command_queue_cb
    );

    /*
     * Create static event group.
     */
    system_events = xEventGroupCreateStatic(
        &system_events_cb
    );

    /*
     * Initialize timestamps.
     *
     * They start at the current system tick.
     *
     * The safety system will therefore detect missing data after the
     * configured timeout.
     */
    last_imu_update = xTaskGetTickCount();
    last_encoder_update = xTaskGetTickCount();

    /*
     * No command has been received yet.
     *
     * Starting with "now" means the communication timeout will
     * eventually force the system into SAFE if no command arrives.
     */
    last_command_update = xTaskGetTickCount();

    /*
     * Clear diagnostics.
     */
    diagnostics.command_queue_overflows = 0U;
    diagnostics.imu_read_errors = 0U;
    diagnostics.encoder_read_errors = 0U;
    diagnostics.imu_timeouts = 0U;
    diagnostics.encoder_timeouts = 0U;
    diagnostics.communication_timeouts = 0U;
    diagnostics.safety_stops = 0U;
}


/*==========================================================================*/
/*                         TASK CREATION                                    */
/*==========================================================================*/

/*
 * Create all application tasks using static allocation.
 *
 * Task priority policy:
 *
 *     Safety       5
 *     Control      4
 *     IMU          3
 *     Encoder      3
 *     Communications 2
 *     Telemetry    1
 *
 * Higher priority = more important/urgent.
 */
void rover_create_tasks(void)
{
    safety_task_handle = xTaskCreateStatic(
        safety_task,
        "Safety",
        ROVER_SAFETY_STACK_SIZE,
        NULL,
        ROVER_SAFETY_TASK_PRIORITY,
        safety_stack,
        &safety_tcb
    );

    control_task_handle = xTaskCreateStatic(
        control_task,
        "Control",
        ROVER_CONTROL_STACK_SIZE,
        NULL,
        ROVER_CONTROL_TASK_PRIORITY,
        control_stack,
        &control_tcb
    );

    imu_task_handle = xTaskCreateStatic(
        imu_task,
        "IMU",
        ROVER_IMU_STACK_SIZE,
        NULL,
        ROVER_IMU_TASK_PRIORITY,
        imu_stack,
        &imu_tcb
    );

    encoder_task_handle = xTaskCreateStatic(
        encoder_task,
        "Encoder",
        ROVER_ENCODER_STACK_SIZE,
        NULL,
        ROVER_ENCODER_TASK_PRIORITY,
        encoder_stack,
        &encoder_tcb
    );

    comms_task_handle = xTaskCreateStatic(
        communications_task,
        "microROS",
        ROVER_COMMS_STACK_SIZE,
        NULL,
        ROVER_COMMS_TASK_PRIORITY,
        comms_stack,
        &comms_tcb
    );

    telemetry_task_handle = xTaskCreateStatic(
        telemetry_task,
        "Telemetry",
        ROVER_TELEMETRY_STACK_SIZE,
        NULL,
        ROVER_TELEMETRY_TASK_PRIORITY,
        telemetry_stack,
        &telemetry_tcb
    );
}


/*==========================================================================*/
/*                         IMU ISR HANDLER                                  */
/*==========================================================================*/

/*
 * Called from the actual STM32 IMU interrupt handler.
 *
 * IMPORTANT:
 *
 * This function must execute only FreeRTOS FromISR APIs.
 *
 * No:
 *     - HAL blocking communication
 *     - printf()
 *     - dynamic allocation
 *     - long calculations
 *     - sensor processing
 *
 * The ISR simply wakes the IMU task.
 */
void rover_imu_data_ready_isr(void)
{
    BaseType_t higher_priority_task_woken;

    higher_priority_task_woken = pdFALSE;

    /*
     * Wake the IMU task.
     */
    if (imu_task_handle != NULL)
    {
        vTaskNotifyGiveFromISR(
            imu_task_handle,
            &higher_priority_task_woken
        );
    }

    /*
     * Request a context switch if the IMU task has a higher priority
     * than the interrupted task.
     */
    portYIELD_FROM_ISR(
        higher_priority_task_woken
    );
}
