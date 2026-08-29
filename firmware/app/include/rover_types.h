#ifndef ROVER_TYPES_H
#define ROVER_TYPES_H
#include <stdint.h>
#include <stdbool.h>
typedef struct { float ax, ay, az, gx, gy, gz; uint32_t timestamp_ms; } rover_imu_t;
typedef struct { int32_t left_ticks, right_ticks; float left_rad_s, right_rad_s; uint32_t timestamp_ms; } rover_encoder_t;
typedef struct { float linear_mps, angular_rps; uint32_t received_at_ms; } rover_command_t;
typedef struct { float battery_v, board_temp_c; uint32_t heartbeat, fault_flags; uint8_t mode; } rover_status_t;
enum { ROVER_MODE_BOOT=0, ROVER_MODE_STANDBY=1, ROVER_MODE_ACTIVE=2, ROVER_MODE_SAFE=3 };
enum { ROVER_FAULT_NONE=0, ROVER_FAULT_COMMAND_TIMEOUT=1u<<0, ROVER_FAULT_LOW_BATTERY=1u<<1,
       ROVER_FAULT_OVERTEMP=1u<<2, ROVER_FAULT_SENSOR=1u<<3, ROVER_FAULT_LINK=1u<<4 };
#endif
