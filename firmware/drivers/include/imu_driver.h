/*
 * ============================================================================
 * IMU Driver Interface
 * ============================================================================
 *
 * File:
 *     firmware/drivers/include/imu_driver.h
 *
 * Purpose:
 *     Hardware-independent interface for the rover IMU.
 *
 * Architecture:
 *
 *     Application
 *          |
 *          v
 *     imu_driver.h
 *          |
 *          v
 *     imu_driver.c
 *          |
 *          v
 *     STM32 HAL / I2C / SPI
 *
 * The application should NOT directly access:
 *
 *     - HAL_I2C_*
 *     - HAL_SPI_*
 *     - IMU registers
 *     - sensor-specific configuration
 *
 * ============================================================================
 */

#ifndef IMU_DRIVER_H
#define IMU_DRIVER_H

#include <stdbool.h>
#include <stdint.h>


/* ============================================================================
 * IMU Data Types
 * ========================================================================== */

/**
 * @brief Raw/converted IMU measurement.
 *
 * Accelerometer:
 *     m/s^2
 *
 * Gyroscope:
 *     rad/s
 *
 * Timestamp:
 *     milliseconds since system start
 */
typedef struct
{
    float ax_mps2;
    float ay_mps2;
    float az_mps2;

    float gx_rps;
    float gy_rps;
    float gz_rps;

    uint32_t timestamp_ms;

} imu_data_t;


/**
 * @brief IMU driver status.
 */
typedef enum
{
    IMU_STATUS_OK = 0,
    IMU_STATUS_NOT_INITIALIZED,
    IMU_STATUS_COMMUNICATION_ERROR,
    IMU_STATUS_INVALID_DATA,
    IMU_STATUS_TIMEOUT

} imu_status_t;


/* ============================================================================
 * Driver Lifecycle
 * ========================================================================== */

/**
 * @brief Initialize the IMU hardware.
 *
 * Performs:
 *
 *     - Bus initialization/check
 *     - Device identification
 *     - Sensor configuration
 *     - Calibration/configuration validation
 *
 * @return true if initialization succeeds.
 */
bool imu_driver_init(void);


/**
 * @brief Deinitialize the IMU.
 *
 * Places the sensor into a safe/inactive state.
 */
void imu_driver_deinit(void);


/* ============================================================================
 * Data Acquisition
 * ========================================================================== */

/**
 * @brief Read the latest IMU measurement.
 *
 * This function should be non-blocking or have a bounded execution time.
 *
 * @param[out] data Destination measurement structure.
 *
 * @return true when a valid measurement was obtained.
 */
bool imu_driver_read(
    imu_data_t *data
);


/**
 * @brief Check whether the IMU currently reports valid data.
 *
 * @return true when the IMU is healthy.
 */
bool imu_driver_is_healthy(void);


/* ============================================================================
 * Interrupt Support
 * ========================================================================== */

/**
 * @brief Notify the driver that the IMU data-ready interrupt occurred.
 *
 * This function is intended to be called from the ISR path.
 *
 * IMPORTANT:
 *
 *     Do not perform I2C/SPI transactions inside the ISR.
 *
 * The ISR should only notify the driver/application task.
 */
void imu_driver_data_ready_isr(void);


/* ============================================================================
 * Diagnostics
 * ========================================================================== */

/**
 * @brief Return the most recent driver status.
 */
imu_status_t imu_driver_get_status(void);


/**
 * @brief Return the number of successful samples.
 */
uint32_t imu_driver_get_sample_count(void);


/**
 * @brief Return the number of communication errors.
 */
uint32_t imu_driver_get_error_count(void);


#endif /* IMU_DRIVER_H */
