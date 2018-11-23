/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_err.h"
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

#include "console.h"
//#include "hal/radio/bluetooth.h"
#include "hal/radio/wifi.h"
#include "jolt_helpers.h"
#include "jolt_globals.h"
#include "hal/i2c.h"
#include "hal/storage/storage.h"
#include "hal/hw_monitor.h"
#include "syscore/filesystem.h"
#include "vault.h"

const jolt_version_t JOLT_VERSION = {
    .major = 0,
    .minor = 1,
    .patch = 0,
    .release = JOLT_VERSION_DEV
};

static QueueHandle_t input_queue;
ssd1306_t disp_hal;

static const char TAG[] = "main";

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

    disp_hal.protocol  = SSD1306_PROTO_I2C;
    disp_hal.screen    = SSD1306_SCREEN;
    disp_hal.i2c_dev   = CONFIG_JOLT_DISPLAY_ADDRESS;
    disp_hal.rst_pin   = CONFIG_JOLT_DISPLAY_PIN_RST;
    disp_hal.width     = LV_HOR_RES;
    disp_hal.height    = LV_VER_RES;
    ESP_ERROR_CHECK(ssd1306_init(&disp_hal));

    /*inverse screen (180°) */
#if CONFIG_JOLT_DISPLAY_FLIP
    ESP_LOGI(TAG, "Flipping Display");
    ssd1306_set_scan_direction_fwd(&disp_hal, true);
    ssd1306_set_segment_remapping_enabled(&disp_hal, false);
#else
    ssd1306_set_scan_direction_fwd(&disp_hal, false);
    ssd1306_set_segment_remapping_enabled(&disp_hal, true);
#endif

    static lv_disp_drv_t lv_disp_drv;
    lv_disp_drv_init(&lv_disp_drv);
    lv_disp_drv.disp_flush = ssd1306_flush;
    lv_disp_drv.vdb_wr = ssd1306_vdb_wr;
    lv_disp_drv_register(&lv_disp_drv);

    ssd1306_set_whole_display_lighting(&disp_hal, false);
    ssd1306_set_inversion(&disp_hal, true);
    ssd1306_set_contrast(&disp_hal, get_display_brightness());
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
    easy_input_queue_init(&input_queue);
    easy_input_run( &input_queue );

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
    ESP_LOGI(TAG, "Starting draw loop");
    for( ;; vTaskDelay( 1 ) ) {
        jolt_gui_sem_take();
        lv_tick_inc(portTICK_RATE_MS);
        lv_task_handler();
        jolt_gui_sem_give();
    }
    ESP_LOGE(TAG, "Draw Loop Exitted"); // Should never reach here
    abort();
}

#ifndef UNIT_TESTING
// So our app_main() doesn't override the unit test app_main()
void app_main() {
    /* Setup and Install I2C Driver and supporting objects */
    esp_err_t err;
    err = i2c_driver_setup();
    if( ESP_OK != err) {
        ESP_LOGE(TAG, "Failed to install i2c driver");
    }

    /* Initialize LVGL graphics system */
    lv_init();
    display_init();
    jolt_gui_store.mutex = xSemaphoreCreateMutex();
    indev_init();

    /* Run Key/Value Storage Initialization */
    storage_startup();

    // Initialize Wireless
    /* todo: this must be before first_boot_setup otherwise attempting
     * to get ap_info before initializing wifi causes a boot loop. investigate
     * more robust solutions */
    esp_log_level_set("wifi", ESP_LOG_NONE);
    set_jolt_cast();
    wifi_connect();

    //jolt_bluetooth_setup();

    // Allocate space for the vault and see if a copy exists in NVS
    jolt_gui_store.first_boot = ( false == vault_setup() );

    // ==== Initialize the file system ====
    filesystem_init();

    /* Create GUI */
    ESP_LOGI(TAG, "Creating GUI");
    jolt_gui_menu_home_create();

    ESP_LOGI(TAG, "Starting Hardware Monitors");
    xTaskCreate(jolt_hw_monitor_task,
            "HW_Monitor", CONFIG_JOLT_TASK_STACK_SIZE_HW_MONITORS,
            NULL, CONFIG_JOLT_TASK_PRIORITY_HW_MONITORS, NULL);

    // Initiate Console
    initialize_console();
    start_console();

    BaseType_t ret;
    ESP_LOGI(TAG, "Creating Screen Draw Task");
    ret = xTaskCreate(littlevgl_task,
                "DrawTask", CONFIG_JOLT_TASK_STACK_SIZE_LVGL,
                NULL, CONFIG_JOLT_TASK_PRIORITY_LVGL, NULL);
    if( pdPASS != ret ){
        if( errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY == ret ) {
            ESP_LOGE(TAG, "%s Couldn't allocate memory for Screen Drawing Task",
                    __func__);
        }
        else {
            ESP_LOGE(TAG, "%s Failed to start drawing task, error_code=%d",
                    __func__, ret);
        }
    }
}
#endif
