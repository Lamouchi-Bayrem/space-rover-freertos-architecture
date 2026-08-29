
#ifndef ROVER_TYPES_H
#define ROVER_TYPES_H

#include <stdbool.h>
#include <stdint.h>

/*
 * ==========================================================================
 * Rover Operating Modes
 * ==========================================================================
 *
 * State progression:
 *
 *      BOOT
 *        |
 *        v
 *      SAFE
 *        |
 *        v
 *      READY
 *        |
 *        v
 *      ACTIVE
 *
 * Any critical fault can force the rover back to SAFE.
 */

typedef enum
{
    ROVER_MODE_BOOT = 0U,
    ROVER_MODE_SAFE,
    ROVER_MODE_READY,
    ROVER_MODE_ACTIVE,
    ROVER_MODE_FAULT

} rover_mode_t;


/*
 * ==========================================================================
 * IMU Measurement
 * ==========================================================================
 */

typedef struct
{
    float ax;
    float ay;
    float az;

    float gx;
    float gy;
    float gz;

    uint32_t timestamp_ms;

} rover_imu_t;


/*
 * ==========================================================================
 * Encoder Measurement
 * ==========================================================================
 */

typedef struct
{
    int32_t left_ticks;
    int32_t right_ticks;

    float left_rad_s;
    float right_rad_s;

    uint32_t timestamp_ms;

} rover_encoder_t;


/*
 * ==========================================================================
 * Rover Command
 * ==========================================================================
 *
 * received_at_ms is the timestamp of the last valid command.
 *
 * The safety supervisor uses it to detect command timeout.
 */

typedef struct
{
    float linear_mps;
    float angular_rps;

    uint32_t received_at_ms;

} rover_command_t;


/*
 * ==========================================================================
 * Rover Fault Flags
 * ==========================================================================
 *
 * Multiple faults can be active simultaneously.
 *
 * Example:
 *
 *     ROVER_FAULT_LOW_BATTERY |
 *     ROVER_FAULT_OVERTEMP
 */

#define ROVER_FAULT_NONE              (0UL)
#define ROVER_FAULT_COMMAND_TIMEOUT   (1UL << 0U)
#define ROVER_FAULT_LOW_BATTERY       (1UL << 1U)
#define ROVER_FAULT_OVERTEMP          (1UL << 2U)
#define ROVER_FAULT_SENSOR            (1UL << 3U)
#define ROVER_FAULT_LINK              (1UL << 4U)


/*
 * ==========================================================================
 * Rover Status
 * ==========================================================================
 */

typedef struct
{
    rover_mode_t mode;

    float battery_v;
    float board_temp_c;

    uint32_t heartbeat;
    uint32_t fault_flags;

} rover_status_t;


#endif /* ROVER_TYPES_H */

