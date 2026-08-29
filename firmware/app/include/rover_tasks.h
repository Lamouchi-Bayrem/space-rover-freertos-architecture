### `firmware/app/include/rover_tasks.h`

```c
#ifndef ROVER_TASKS_H
#define ROVER_TASKS_H

#include <stdint.h>

/*
 * ==========================================================================
 * Rover Application Initialization
 * ==========================================================================
 *
 * rover_app_init()
 *     Initializes application state, HAL and static FreeRTOS objects.
 *
 * rover_create_tasks()
 *     Creates all rover application tasks.
 *
 * These functions must be called before starting the FreeRTOS scheduler.
 */

void rover_app_init(void);

void rover_create_tasks(void);


/*
 * ==========================================================================
 * Interrupt Interface
 * ==========================================================================
 *
 * Called by the STM32 IMU interrupt handler when new IMU data is available.
 *
 * The function must remain ISR-safe:
 *
 *     - No blocking operations
 *     - No printf()
 *     - No dynamic memory allocation
 *     - No normal FreeRTOS task API
 *
 * Only FromISR APIs should be used internally.
 */
void rover_imu_data_ready_isr(void);


#endif /* ROVER_TASKS_H */
```
