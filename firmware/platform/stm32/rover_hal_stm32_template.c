/* Copy into the STM32 target and replace every TODO with HAL/BSP code. */
#include "rover_hal.h"
void rover_hal_init(void) { /* TODO */ }
uint32_t rover_hal_millis(void) { return 0; /* HAL_GetTick() */ }
bool rover_hal_imu_read(rover_imu_t *s) { (void)s; return false; }
bool rover_hal_encoder_read(rover_encoder_t *s) { (void)s; return false; }
float rover_hal_battery_voltage(void) { return 0.0f; }
float rover_hal_board_temperature(void) { return 0.0f; }
void rover_hal_motor_command(float l,float a) { (void)l;(void)a; }
void rover_hal_motor_stop(void) { }
void rover_hal_debug_status(const rover_status_t *s) { (void)s; }
