#ifndef ENCODER_DRIVER_H
#define ENCODER_DRIVER_H
#include "rover_types.h"
#include <stdbool.h>
bool encoder_driver_init(void);
bool encoder_driver_read(rover_encoder_t *sample);
#endif
