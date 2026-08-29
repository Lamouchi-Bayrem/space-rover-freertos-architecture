#ifndef ROVER_HAL_H
#define ROVER_HAL_H
#include <stdbool.h>
#include <stdint.h>
#include "rover_types.h"
void rover_hal_init(void);
uint32_t rover_hal_millis(void);
bool rover_hal_imu_read(rover_imu_t *sample);
bool rover_hal_encoder_read(rover_encoder_t *sample);
float rover_hal_battery_voltage(void);
float rover_hal_board_temperature(void);
void rover_hal_motor_command(float linear_mps, float angular_rps);
void rover_hal_motor_stop(void);
void rover_hal_debug_status(const rover_status_t *status);
#endif
