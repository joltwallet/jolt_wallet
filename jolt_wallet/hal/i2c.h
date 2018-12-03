#ifndef __JOLT_HAL_I2C_H__
#define __JOLT_HAL_I2C_H__

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

int i2c_driver_setup();

/* Semaphore is declared here to reduce circular dependencies */
SemaphoreHandle_t i2c_sem;
#define I2C_SEM_TAKE xSemaphoreTake( i2c_sem, portMAX_DELAY );
#define I2C_SEM_GIVE xSemaphoreGive( i2c_sem );

#endif
