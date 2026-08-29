#include "rover_state.h"
#include "rover_safety.h"
#include "rover_hal.h"
#include <stdio.h>
int main(void) {
  rover_hal_init(); rover_state_init();
  rover_command_t c={.linear_mps=.2f,.received_at_ms=100}; rover_state_set_command(&c);
  uint32_t f=rover_safety_evaluate(200,&c,rover_hal_battery_voltage(),rover_hal_board_temperature(),true,true);
  printf("host demo faults=0x%08x
",f); return f?1:0;
}
