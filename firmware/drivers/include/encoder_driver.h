### `firmware/drivers/include/encoder_driver.h`

```c
/*
 * ============================================================================
 * Encoder Driver Interface
 * ============================================================================
 *
 * File:
 *     firmware/drivers/include/encoder_driver.h
 *
 * Purpose:
 *     Hardware-independent interface for the rover wheel encoders.
 *
 * Architecture:
 *
 *     Application
 *          |
 *          v
 *     encoder_driver.h
 *          |
 *          v
 *     encoder_driver.c
 *          |
 *          v
 *     STM32 HAL / TIM / GPIO
 *
 * ============================================================================
 */

#ifndef ENCODER_DRIVER_H
#define ENCODER_DRIVER_H

#include <stdbool.h>
#include <stdint.h>


/* ============================================================================
 * Encoder Data
 * ========================================================================== */

/**
 * @brief Encoder measurement.
 *
 * left_ticks/right_ticks:
 *     Accumulated encoder counts.
 *
 * left_rad_s/right_rad_s:
 *     Estimated wheel angular velocity.
 *
 * timestamp_ms:
 *     Measurement timestamp.
 */
typedef struct
{
    int32_t left_ticks;
    int32_t right_ticks;

    float left_rad_s;
    float right_rad_s;

    uint32_t timestamp_ms;

} encoder_data_t;


/**
 * @brief Encoder driver status.
 */
typedef enum
{
    ENCODER_STATUS_OK = 0,
    ENCODER_STATUS_NOT_INITIALIZED,
    ENCODER_STATUS_INVALID_DATA,
    ENCODER_STATUS_TIMEOUT

} encoder_status_t;


/* ============================================================================
 * Driver Lifecycle
 * ========================================================================== */

/**
 * @brief Initialize the encoder driver.
 *
 * Configures the required timers/GPIO interfaces and resets
 * the internal measurement state.
 *
 * @return true when initialization succeeds.
 */
bool encoder_driver_init(void);


/**
 * @brief Deinitialize the encoder driver.
 */
void encoder_driver_deinit(void);


/* ============================================================================
 * Data Acquisition
 * ========================================================================== */

/**
 * @brief Read the latest encoder measurement.
 *
 * @param[out] data Destination measurement structure.
 *
 * @return true when valid data is available.
 */
bool encoder_driver_read(
    encoder_data_t *data
);


/**
 * @brief Reset encoder counters.
 *
 * Useful during startup or controlled reinitialization.
 */
void encoder_driver_reset(void);


/* ============================================================================
 * Health Monitoring
 * ========================================================================== */

/**
 * @brief Check encoder health.
 *
 * @return true when encoder data is valid.
 */
bool encoder_driver_is_healthy(void);


/**
 * @brief Return the latest driver status.
 */
encoder_status_t encoder_driver_get_status(void);


/* ============================================================================
 * Diagnostics
 * ========================================================================== */

/**
 * @brief Return the number of encoder read operations.
 */
uint32_t encoder_driver_get_sample_count(void);


/**
 * @brief Return the number of detected encoder errors.
 */
uint32_t encoder_driver_get_error_count(void);


#endif /* ENCODER_DRIVER_H */
```
