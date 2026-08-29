#include "rover_safety.h"
#include "rover_config.h"
uint32_t rover_safety_evaluate(uint32_t now, const rover_command_t *cmd, float battery, float temp, bool sensors_ok, bool link_ok) {
  uint32_t f=ROVER_FAULT_NONE;
  if (!cmd || (uint32_t)(now-cmd->received_at_ms)>ROVER_COMMAND_TIMEOUT_MS) f|=ROVER_FAULT_COMMAND_TIMEOUT;
  if (battery<ROVER_LOW_BATTERY_V) f|=ROVER_FAULT_LOW_BATTERY;
  if (temp>ROVER_MAX_BOARD_TEMP_C) f|=ROVER_FAULT_OVERTEMP;
  if (!sensors_ok) f|=ROVER_FAULT_SENSOR;
  if (!link_ok) f|=ROVER_FAULT_LINK;
  return f;
}
bool rover_safety_requires_stop(uint32_t faults) { return faults!=ROVER_FAULT_NONE; }
