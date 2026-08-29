#include "rover_hal.h"
#include <stdio.h>
#include <time.h>
void rover_hal_init(void) { puts("host HAL initialized"); }
uint32_t rover_hal_millis(void) { return (uint32_t)((clock()*1000u)/CLOCKS_PER_SEC); }
bool rover_hal_imu_read(rover_imu_t *s) { if(!s)return false; *s=(rover_imu_t){.az=9.81f,.timestamp_ms=rover_hal_millis()}; return true; }
bool rover_hal_encoder_read(rover_encoder_t *s) { if(!s)return false; *s=(rover_encoder_t){.timestamp_ms=rover_hal_millis()}; return true; }
float rover_hal_battery_voltage(void) { return 12.2f; }
float rover_hal_board_temperature(void) { return 35.0f; }
void rover_hal_motor_command(float l,float a) { printf("motor command %.2f %.2f
",l,a); }
void rover_hal_motor_stop(void) { puts("motor stop"); }
void rover_hal_debug_status(const rover_status_t *s) { printf("mode=%u faults=0x%08x battery=%.2fV
",s->mode,s->fault_flags,s->battery_v); }
