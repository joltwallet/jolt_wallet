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
#include "esp_event_loop.h"

#include <driver/adc.h>
#include "esp_adc_cal.h"

#include "easy_input.h"

#include "lv_conf.h"
#include "lvgl/lvgl.h"
#include "jolt_gui/jolt_gui.h"

#include "console.h"
#include "hal/radio/bluetooth.h"
#include "hal/radio/wifi.h"
#include "hal/display.h"
#include "hal/i2c.h"
#include "hal/storage/storage.h"
#include "hal/hw_monitor.h"
#include "syscore/filesystem.h"
#include "vault.h"

#include "esp_ota_ops.h"

#include "jolt_lib.h"

#if CONFIG_HEAP_TRACING
#include "esp_heap_trace.h"
#define HEAP_TRACING_NUM_RECORDS 100
static heap_trace_record_t trace_records[HEAP_TRACING_NUM_RECORDS];
#endif

const jolt_version_t JOLT_VERSION = {
    .major = 0,
    .minor = 1,
    .patch = 0,
    .release = JOLT_VERSION_DEV
};

static QueueHandle_t input_queue;
static const char TAG[] = "main";

static IRAM_ATTR bool easy_input_read(lv_indev_data_t *data) {
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
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for( ;; vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(10) ) ) {
        JOLT_GUI_CTX{
            lv_task_handler();
        }
    }
    ESP_LOGE(TAG, "Draw Loop Exitted"); // Should never reach here
    abort();
}

#ifndef UNIT_TESTING
// So our app_main() doesn't override the unit test app_main()
void app_main() {
    /* Setup Heap Logging */
    #if CONFIG_HEAP_TRACING
    {
        ESP_ERROR_CHECK( heap_trace_init_standalone(trace_records,
                    HEAP_TRACING_NUM_RECORDS) );
    }
    #endif
    /* Check currently running partition */
    {
        const esp_partition_t *partition = esp_ota_get_running_partition();
        ESP_LOGI(TAG, "Currently Running %s at 0x%08X.",
                partition->label, partition->address);
        ESP_LOGI(TAG, "Partition is %sencrypted.",
                partition->encrypted ? "" : "not ");
    }

    /* Setup and Install I2C Driver and supporting objects */
    esp_err_t err;
    err = i2c_driver_setup();
    if( ESP_OK != err) {
        ESP_LOGE(TAG, "Failed to install i2c driver");
    }

    /* Initialize LVGL graphics system */
    lv_init();
    display_init();
    indev_init();

    /* These lines are for configuring the ADC for sensing battery voltage;
     * refactor these to be somewhere else later */
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(JOLT_ADC1_VBATT, ADC_ATTEN_DB_11);

    /* Run Key/Value Storage Initialization */
    storage_startup();

    /* Start Event Loop */
#if !CONFIG_NO_BLOBS
    /* todo: this probably shouldn't be so tightly coupled with WiFi */
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
#endif

    // Initialize Wireless
    /* todo: this must be before first_boot_setup otherwise attempting
     * to get ap_info before initializing wifi causes a boot loop. investigate
     * more robust solutions */
    esp_log_level_set("wifi", ESP_LOG_NONE);
    set_jolt_cast();
    {
        uint8_t wifi_en;
        storage_get_u8(&wifi_en, "user", "wifi_en", 0 );
        if( wifi_en ) {
            wifi_connect();
        }
    }

    // Allocate space for the vault and see if a copy exists in NVS
    jolt_gui_store.first_boot = ( false == vault_setup() );

    // ==== Initialize the file system ====
    jolt_fs_init();

    /* Create GUI */
    {
        ESP_LOGI(TAG, "Creating GUI");
        jolt_lang_t lang;
        storage_get_u8(&lang, "user", "lang", CONFIG_JOLT_LANG_DEFAULT );
        jolt_lang_set( lang ); // Internally initializes the theme
    }

    lv_obj_t *btn_back = lv_btn_create(lv_scr_act(), NULL);
    lv_btn_set_action(btn_back, LV_BTN_ACTION_CLICK, jolt_gui_scr_del);
    lv_group_add_obj(jolt_gui_store.group.back, btn_back);

    /* Create StatusBar */
    statusbar_create();

    if( jolt_gui_store.first_boot ) {
        /* Create First Boot Screen */
        jolt_gui_first_boot_create();
    }
    else{
        /* Create Home Menu */
        jolt_gui_menu_home_create();
    }

    ESP_LOGI(TAG, "Starting Hardware Monitors");
    xTaskCreate(jolt_hw_monitor_task,
            "HW_Monitor", CONFIG_JOLT_TASK_STACK_SIZE_HW_MONITORS,
            NULL, CONFIG_JOLT_TASK_PRIORITY_HW_MONITORS, NULL);

    // Initiate Console
    {
        uint8_t bluetooth_en;
        storage_get_u8(&bluetooth_en, "user", "bluetooth_en", 0 );
        if( bluetooth_en ) {
            jolt_bluetooth_setup(); // starts a task adding bluetooth commands to the command queue
        }
    }

    console_init();
    console_start(); // starts a task adding uart commands to the command queue. Also starts the task to process the command queue.

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
