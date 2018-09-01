/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "driver/i2c.h"

#include "u8g2.h"
#include "menu8g2.h"
#include "easy_input.h"
#include "aes132_comm_marshaling.h" // todo remove this; for debugging only

#include "gui/gui.h"
#include "radio/wifi.h"
#include "gui/first_boot.h"
#include "vault.h"
#include "globals.h"
#include "console.h"
#include "helpers.h"
#include "syscore/filesystem.h"
#include "gui/statusbar.h"


// Definitions for variables in globals.h
volatile u8g2_t u8g2_obj;
volatile u8g2_t *u8g2;
volatile QueueHandle_t input_queue;
volatile SemaphoreHandle_t disp_mutex;
volatile menu8g2_t menu_obj;
volatile menu8g2_t *menu;

static const char TAG[] = "main";

void printf_puthex_array(uint8_t* data_buffer, uint8_t length) {
	uint8_t i_data;
    printf("Response: ");
	for (i_data = 0; i_data < length; i_data++) {
		printf("%.2X",*data_buffer++);
		printf(" ");
	}
    printf("\n");
}

void app_main(){
    // Setup Input Button Debouncing Code
    easy_input_queue_init((QueueHandle_t *)&input_queue);
    xTaskCreate(easy_input_push_button_task,
            "ButtonDebounce", 2500,
            (void *)&input_queue, 19,
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
    ESP_LOGI(TAG, "i2c_driver_install %d", CONFIG_JOLT_I2C_MASTER_NUM);
    ESP_ERROR_CHECK(i2c_driver_install(CONFIG_JOLT_I2C_MASTER_NUM, conf.mode, 0, 0, 0));

    {
        // testing ataes132a chip; remove after done testing.
        // internally, aes132m_execute concatenates all the datablocks into tx_buffer
        uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
        uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};
        uint8_t res = aes132m_execute(AES132_RANDOM, 0x01, 0x0000, 0x0000,
			0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
        printf_puthex_array(rx_buffer, sizeof(rx_buffer));

        return;
    }
    // Initialize the OLED Display
    u8g2 = &u8g2_obj;
    setup_screen((u8g2_t *) u8g2);
    u8g2_SetContrast( u8g2, get_display_brightness() );
    disp_mutex = xSemaphoreCreateMutex();

    // Create Global Menu Object
    menu = &menu_obj;
    menu8g2_init(menu, (u8g2_t *) u8g2, input_queue, disp_mutex, NULL, statusbar_update);

    // Allocate space for the vault and see if a copy exists in NVS
    if( false == vault_setup()) {
        first_boot_menu();
    }
    
    // Initialize Wireless
    esp_log_level_set("wifi", ESP_LOG_NONE);
    set_jolt_cast();
    wifi_connect();

    // todo: remove this, just here for sanity checks;
    // turn down main task stack back to 3584 when done debugging
#if 0
    vTaskDelay( 5000 / portTICK_PERIOD_MS );
    char buf[1024];
    network_get_data( (unsigned char *)"{ \"action\" : \"block_count\"}",
            (unsigned char *)buf, sizeof(buf) );
    printf("%s\n", buf);
    return;
#endif
    
    xTaskCreate(gui_task,
            "GUI", 16000,
            NULL, 10,
            NULL);

    // ==== Initialize the file system ====
    filesystem_init();

    // Initiate Console
    initialize_console();
    start_console();
}
