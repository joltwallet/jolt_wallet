/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "nvs.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "driver/i2c.h"

#include "u8g2.h"
#include "menu8g2.h"
#include "easy_input.h"
#include "nano_lws.h"

#include "gui/gui.h"
#include "radio/wifi.h"
#include "gui/first_boot.h"
#include "vault.h"
#include "globals.h"
#include "console.h"
#include "helpers.h"
#include "syscore/filesystem.h"


// Definitions for variables in globals.h
volatile u8g2_t u8g2;
volatile QueueHandle_t input_queue;
volatile QueueHandle_t vault_queue;
volatile SemaphoreHandle_t disp_mutex;
QueueHandle_t backend_queue;

static const char TAG[] = "main";

void app_main(){
    // Setup Input Button Debouncing Code
    easy_input_queue_init((QueueHandle_t *)&input_queue);
    xTaskCreate(easy_input_push_button_task,
            "ButtonDebounce", 2500,
            (void *)&input_queue, 15,
            NULL);

    // Setup and Install I2C Driver
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
    ESP_ERROR_CHECK(i2c_param_config(CONFIG_JOLT_I2C_MASTER_NUM, &conf));
    ESP_LOGI(TAG, "i2c_driver_install %d", I2C_NUM_1);
    ESP_ERROR_CHECK(i2c_driver_install(CONFIG_JOLT_I2C_MASTER_NUM, conf.mode, 0, 0, 0));

    // Initialize the OLED Display
    setup_screen((u8g2_t *) &u8g2);
    u8g2_SetContrast( &u8g2, get_display_brightness() );
    disp_mutex = xSemaphoreCreateMutex();
    
    // Allocate space for the vault and see if a copy exists in NVS
    vault_t vault;
    if (E_FAILURE == vault_init(&vault)){
        first_boot_menu();
    }
    
    // Initialize Wireless
    set_jolt_cast();
    wifi_connect();

    xTaskCreate(vault_task,
            "Vault", 16000,
            (void *) &vault, 16,
            NULL);
    
    xTaskCreate(gui_task,
            "GUI", 16000,
            NULL, 10,
            NULL);

    xTaskCreate(network_task,
            "Network", 3200,
            NULL, 5,
            NULL);

    // ==== Initialize the file system ====
    filesystem_init();

    // Initiate Console
    initialize_console();
    start_console();
}
