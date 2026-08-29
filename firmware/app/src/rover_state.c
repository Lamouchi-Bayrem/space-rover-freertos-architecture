#include "rover_state.h"
static rover_command_t command_state;
static rover_status_t status_state;
void rover_state_init(void) { command_state=(rover_command_t){0}; status_state=(rover_status_t){0}; status_state.mode=ROVER_MODE_BOOT; }
void rover_state_set_command(const rover_command_t *v) { if(v) command_state=*v; }
rover_command_t rover_state_get_command(void) { return command_state; }
void rover_state_set_status(const rover_status_t *v) { if(v) status_state=*v; }
rover_status_t rover_state_get_status(void) { return status_state; }
