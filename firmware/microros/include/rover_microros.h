```c
/*
 * ============================================================================
 * Rover micro-ROS Interface
 * ============================================================================
 *
 * File:
 *     firmware/microros/include/rover_microros.h
 *
 * Purpose:
 *     Interface between the rover application and micro-ROS.
 *
 * Responsibilities:
 *
 *     - Initialize micro-ROS entities
 *     - Run the micro-ROS executor
 *     - Publish rover telemetry
 *     - Receive velocity commands
 *     - Handle rover mode requests
 *
 * Architecture:
 *
 *     ROS 2
 *       |
 *       v
 *     micro-ROS Agent
 *       |
 *       v
 *     micro-ROS transport
 *       |
 *       v
 *     rover_microros.c
 *       |
 *       +----> rover_state
 *       |
 *       +----> rover_tasks
 *       |
 *       +----> rover safety
 *
 * IMPORTANT:
 *
 *     The micro-ROS executor runs from ONE FreeRTOS task.
 *
 *     Do not access micro-ROS entities from interrupts.
 *
 * ============================================================================
 */

#ifndef ROVER_MICROROS_H
#define ROVER_MICROROS_H

#include <stdbool.h>
#include <stdint.h>


/* ============================================================================
 * micro-ROS Lifecycle
 * ========================================================================== */

/**
 * @brief Initialize the micro-ROS subsystem.
 *
 * Creates:
 *
 *     - Node
 *     - Publishers
 *     - Subscriber
 *     - Service
 *     - Executor
 *
 * @return true when initialization succeeds.
 */
bool rover_microros_init(void);


/**
 * @brief Deinitialize the micro-ROS subsystem.
 *
 * Releases micro-ROS entities created by rover_microros_init().
 */
void rover_microros_deinit(void);


/**
 * @brief Execute one micro-ROS processing cycle.
 *
 * This function is called by the dedicated micro-ROS FreeRTOS task.
 *
 * It must NOT be called from an ISR.
 *
 * @param timeout_ms Maximum executor wait time.
 */
void rover_microros_spin(
    uint32_t timeout_ms
);


/* ============================================================================
 * Communication Status
 * ========================================================================== */

/**
 * @brief Check whether the micro-ROS connection is healthy.
 *
 * @return true when the communication link is operational.
 */
bool rover_microros_is_connected(void);


/**
 * @brief Return the number of received commands.
 */
uint32_t rover_microros_get_command_count(void);


/**
 * @brief Return the number of communication errors.
 */
uint32_t rover_microros_get_error_count(void);


/* ============================================================================
 * Telemetry
 * ========================================================================== */

/**
 * @brief Publish current rover telemetry.
 *
 * The function obtains application state and publishes:
 *
 *     /rover/imu
 *     /rover/wheel_ticks
 *     /rover/status
 *
 * This function must be called from the micro-ROS task context.
 *
 * @return true when the telemetry publication succeeds.
 */
bool rover_microros_publish_telemetry(void);


/* ============================================================================
 * Command Processing
 * ========================================================================== */

/**
 * @brief Process a received velocity command.
 *
 * The micro-ROS subscriber callback should convert the ROS command into
 * rover_command_t and submit it to the application command interface.
 *
 * IMPORTANT:
 *
 *     The callback should remain short and deterministic.
 */
void rover_microros_process_command(
    float linear_mps,
    float angular_rps
);


#endif /* ROVER_MICROROS_H */
```
