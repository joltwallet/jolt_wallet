
#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "jolttypes.h"

static const char TAG[] = "hal/i2c";

SemaphoreHandle_t i2c_sem;

esp_err_t i2c_driver_setup() {
    i2c_sem = xSemaphoreCreateMutex();

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    ESP_LOGI(TAG, "sda_io_num %d", CONFIG_JOLT_I2C_PIN_SDA);
    conf.sda_io_num = CONFIG_JOLT_I2C_PIN_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    ESP_LOGI(TAG, "scl_io_num %d", CONFIG_JOLT_I2C_PIN_SCL);
    conf.scl_io_num = CONFIG_JOLT_I2C_PIN_SCL;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    ESP_LOGI(TAG, "clk_speed %d", CONFIG_JOLT_I2C_MASTER_FREQ_HZ);
    conf.master.clk_speed = CONFIG_JOLT_I2C_MASTER_FREQ_HZ;
    ESP_LOGI(TAG, "i2c_param_config %d", conf.mode);

    esp_err_t err;
    err = i2c_param_config(CONFIG_JOLT_I2C_MASTER_NUM, &conf);
    if( E_SUCCESS != err ) {
        goto err;
    }
    ESP_LOGI(TAG, "i2c_driver_install %d", CONFIG_JOLT_I2C_MASTER_NUM);

    err = i2c_driver_install(CONFIG_JOLT_I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if( E_SUCCESS != err ) {
        goto err;
    }
    return ESP_OK;

err:
#if CONFIG_JOLT_I2C_ERROR_RESET
        abort();
#endif
    return ESP_FAIL;
}

