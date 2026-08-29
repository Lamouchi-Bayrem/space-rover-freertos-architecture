# STM32CubeMX integration

## Recommended first target

Choose a board you own and can debug with SWD. Configure clock, debug, one UART for logs
or micro-ROS transport, I2C/SPI for the IMU, timer encoder channels, PWM timers, ADC for
battery measurement and a hardware watchdog.

## CubeMX sequence

1. Create the board project and enable serial-wire debugging.
2. Configure peripheral pins, clocks and DMA.
3. Add FreeRTOS and CMSIS-RTOS only if required by your generated environment.
4. Preserve user-code sections during regeneration.
5. Add application include/source paths.
6. Exclude the host HAL and include the STM32 HAL implementation.
7. Copy recommended options into the generated FreeRTOS configuration carefully.
8. Verify interrupt priorities before using FreeRTOS APIs from ISRs.
9. First run with motors electrically disabled.
10. Validate watchdog and safe-state output before closed-loop control.
