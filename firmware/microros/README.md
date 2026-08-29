# micro-ROS target integration

Add generated micro-ROS libraries and transport glue here after selecting the board.
Recommended initial topics:

- Publish: `/rover/imu`, `/rover/wheel_ticks`, `/rover/status`
- Subscribe: `/cmd_vel`
- Service: `/rover/set_mode`

Keep the executor in one task. Do not call normal FreeRTOS APIs from interrupts unless
the API name ends in `FromISR` and its interrupt-priority requirements are satisfied.
