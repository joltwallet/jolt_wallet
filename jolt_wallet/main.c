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
#include "esp_freertos_hooks.h"

#include <driver/adc.h>
#include "esp_adc_cal.h"

#include "easy_input.h"

#include "lv_conf.h"
#include "lvgl/lvgl.h"
#include "hal/lv_drivers/display/ssd1306.h"
#include "jolt_gui/jolt_gui.h"

#if 0
#include "gui/gui.h"
#include "gui/first_boot.h"
#include "gui/statusbar.h"
#endif

#include "console.h"
#include "radio/wifi.h"
#include "jolt_helpers.h" // todo; move jolt cast into wifi?
#include "globals.h"
#include "hal/i2c.h"
#include "hal/storage/storage.h"
#include "hal/hw_monitor.h"
#include "syscore/filesystem.h"
#include "vault.h"


static void lv_tick_task(void);

// Definitions for variables in globals.h
volatile QueueHandle_t input_queue;

static const char TAG[] = "main";

static ssd1306_t disp_conf = {
	.protocol = SSD1306_PROTO_I2C,
	.screen = SSD1306_SCREEN,
	.i2c_dev = CONFIG_JOLT_DISPLAY_ADDRESS,
	.rst_pin = CONFIG_JOLT_DISPLAY_PIN_RST,
	.width = LV_HOR_RES,
	.height = LV_VER_RES
};

static void display_init() {
    /* Set reset pin as output */
	gpio_config_t io_config;
	io_config.pin_bit_mask = (1 << CONFIG_JOLT_DISPLAY_PIN_RST);
	io_config.mode         = GPIO_MODE_OUTPUT;
	io_config.pull_up_en   = GPIO_PULLUP_DISABLE;
	io_config.pull_down_en = GPIO_PULLDOWN_ENABLE;
	io_config.intr_type    = GPIO_INTR_DISABLE;
	ESP_ERROR_CHECK(gpio_config(&io_config));

    /* These lines are for configuring the ADC for sensing battery voltage;
     * refactor these to be somewhere else later */
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(JOLT_ADC1_VBATT, ADC_ATTEN_DB_11);

    ESP_LOGI(TAG, "Disp_conf %p", &disp_conf);
    ssd1306_init(&disp_conf); // todo error handling

    /*inverse screen (180°) */
#if CONFIG_JOLT_DISPLAY_FLIP
    ESP_LOGI(TAG, "Flipping Display");
    ssd1306_set_scan_direction_fwd(&disp_conf, true);
    ssd1306_set_segment_remapping_enabled(&disp_conf, false);
#else
    ssd1306_set_scan_direction_fwd(&disp_conf, false);
    ssd1306_set_segment_remapping_enabled(&disp_conf, true);
#endif

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.disp_flush = ssd1306_flush;
    disp_drv.vdb_wr = ssd1306_vdb_wr;
    lv_disp_drv_register(&disp_drv);
    ssd1306_set_whole_display_lighting(&disp_conf, false);
    ssd1306_set_inversion(&disp_conf, true);
    ssd1306_set_contrast(&disp_conf, 1); // Set brightness to lowest setting; todo: change
}

static bool easy_input_read(lv_indev_data_t *data) {
    data->state = LV_INDEV_STATE_REL;

    uint64_t input_buf;
    if(xQueueReceive(input_queue, &input_buf, 0)) {
        data->state = LV_INDEV_STATE_PR;
		if(input_buf & (1ULL << EASY_INPUT_BACK)){
            ESP_LOGD(TAG, "back");
            lv_group_send_data(jolt_gui_store.group.back, LV_GROUP_KEY_ENTER);
		}
		else if(input_buf & (1ULL << EASY_INPUT_UP)){
            ESP_LOGD(TAG, "up");
            data->key = LV_GROUP_KEY_UP;
		}
		else if(input_buf & (1ULL << EASY_INPUT_DOWN)){
            ESP_LOGD(TAG, "down");
            data->key = LV_GROUP_KEY_DOWN;
		}
		else if(input_buf & (1ULL << EASY_INPUT_ENTER)){
            ESP_LOGD(TAG, "enter");
            lv_group_send_data(jolt_gui_store.group.enter, LV_GROUP_KEY_ENTER);
		}
        else {
        }
        if( xQueuePeek(input_queue, &input_buf, 0) ) {
            return true;
        }
    }

    return false;
}

static void indev_init() {
    /* Setup Input Button Debouncing Code */
    easy_input_queue_init((QueueHandle_t *)&input_queue);
    easy_input_run( (QueueHandle_t *)&input_queue );

    jolt_gui_group_create();

    lv_indev_t *indev;
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);

    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read = easy_input_read;

    indev = lv_indev_drv_register(&indev_drv);
    lv_indev_set_group(indev, jolt_gui_store.group.main);
}


void littlevgl_task() {
    for( uint8_t i = 0;; vTaskDelay(1) ) {
        i = (i+1) % 5;
        lv_tick_inc(portTICK_RATE_MS);
        if( 0 == i ) { // need to call this less frequently
            xSemaphoreTake( jolt_gui_store.mutex, portMAX_DELAY );
            lv_task_handler();
            xSemaphoreGive( jolt_gui_store.mutex );
        }
    }
}

#ifndef UNIT_TESTING
// So our app_main() doesn't override the unit test app_main()
void app_main() {
    /* Setup and Install I2C Driver and supporting objects */
    i2c_driver_setup();

    /* Initialize LVGL graphics system */
    lv_init();
    display_init();
    jolt_gui_store.mutex = xSemaphoreCreateMutex();
    indev_init();

    /* Run Key/Value Storage Initialization */
    storage_startup();

    //u8g2_SetContrast( u8g2, get_display_brightness() );

    // Initialize Wireless
    /* todo: this must be before first_boot_setup otherwise attempting
     * to get ap_info before initializing wifi causes a boot loop. investigate
     * more robust solutions */
    esp_log_level_set("wifi", ESP_LOG_NONE);
    set_jolt_cast();
    wifi_connect();

    // Allocate space for the vault and see if a copy exists in NVS
    jolt_gui_store.first_boot = ( false == vault_setup() );

    // ==== Initialize the file system ====
    filesystem_init();

    /* Register lv_tick_task after initializing filesystem because initial
     * SPIFFS formatting temporarily disables cache */
    /* Moving tick updater into explicit task; esp_register_freertos_tick_hook 
     * causes SPIFFS to fail */
    //esp_register_freertos_tick_hook(lv_tick_task);

    /* Create GUI */

    //Create main screen obj

    ESP_LOGI(TAG, "Creating GUI");
    jolt_gui_menu_home_create();

    ESP_LOGI(TAG, "Starting Hardware Monitors");
    xTaskCreate(jolt_hw_monitor_task,
            "HW_Monitor", 32000, NULL, CONFIG_JOLT_TASK_PRIORITY_HW_MONITORS, NULL);

    // Initiate Console
    initialize_console();
    start_console();

    xTaskCreate(littlevgl_task,
                "DrawTask", 32000,
                NULL, CONFIG_JOLT_TASK_PRIORITY_LVGL, NULL);
}
#endif
