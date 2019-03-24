/**
 * @file i2c.h
 * @brief I2C setup and semaphore
 * @author Brian Pugh
 */

#ifndef __JOLT_HAL_I2C_H__
#define __JOLT_HAL_I2C_H__

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "mp.h"

/**
 * @brief Configures the I2C hardware.
 *
 * Will automatically reset device on failure if CONFIG_JOLT_I2C_ERROR_RESET.
 * This is done on production hardware since the I2C devices are critical for 
 * secure operation.
 *
 * Also initializes the I2C recursive semaphore.
 *
 * @return ESP_OK on success; ESP_FAIL otherwise. 
 */
esp_err_t i2c_driver_setup();

/**
 * @brief Semaphore for I2C bus access
 */
extern SemaphoreHandle_t i2c_sem;

/**
 * @brief Take the I2C semaphore
 */
#define I2C_SEM_TAKE xSemaphoreTakeRecursive( i2c_sem, portMAX_DELAY );

/**
 * @brief Give the I2C semaphore
 */
#define I2C_SEM_GIVE xSemaphoreGiveRecursive( i2c_sem );

/**
 * @brief Context that takes the I2C semaphore in the beginning, and gives it upon completion
 */
#define I2C_CTX \
    MPP_BEFORE(1, I2C_SEM_TAKE ) \
    MPP_DO_WHILE(2, false) \
    MPP_BREAK_HANDLER(3, ESP_LOGE(TAG, "I2C_CTX break L%d", __LINE__)) \
    MPP_FINALLY(4, I2C_SEM_GIVE )


#endif
