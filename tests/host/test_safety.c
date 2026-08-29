#include "rover_safety.h"
#include "rover_config.h"
#include <assert.h>
int main(void) {
 rover_command_t c={.received_at_ms=1000};
 assert(rover_safety_evaluate(1100,&c,12.0f,30.0f,true,true)==0);
 assert(rover_safety_evaluate(2000,&c,12.0f,30.0f,true,true)&ROVER_FAULT_COMMAND_TIMEOUT);
 assert(rover_safety_evaluate(1100,&c,9.0f,30.0f,true,true)&ROVER_FAULT_LOW_BATTERY);
 assert(rover_safety_evaluate(1100,&c,12.0f,90.0f,true,true)&ROVER_FAULT_OVERTEMP);
 assert(rover_safety_requires_stop(ROVER_FAULT_LINK)); return 0;
}
