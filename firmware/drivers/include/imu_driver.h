#ifndef IMU_DRIVER_H
#define IMU_DRIVER_H
#include "rover_types.h"
#include <stdbool.h>
bool imu_driver_init(void);
bool imu_driver_read(rover_imu_t *sample);
#endif
