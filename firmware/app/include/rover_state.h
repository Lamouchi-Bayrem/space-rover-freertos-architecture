#ifndef ROVER_STATE_H
#define ROVER_STATE_H
#include "rover_types.h"
void rover_state_init(void);
void rover_state_set_command(const rover_command_t *command);
rover_command_t rover_state_get_command(void);
void rover_state_set_status(const rover_status_t *status);
rover_status_t rover_state_get_status(void);
#endif
