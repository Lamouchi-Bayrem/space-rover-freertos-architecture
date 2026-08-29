#ifndef ROVER_SAFETY_H
#define ROVER_SAFETY_H
#include "rover_types.h"
uint32_t rover_safety_evaluate(uint32_t now_ms, const rover_command_t *cmd, float battery_v, float temp_c, bool sensors_ok, bool link_ok);
bool rover_safety_requires_stop(uint32_t faults);
#endif
