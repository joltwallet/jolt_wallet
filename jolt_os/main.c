/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */
#include "sdkconfig.h"

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
#include "esp_pm.h"

#include <driver/adc.h>
#include "esp_adc_cal.h"


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
#include "hal/led.h"
#include "syscore/filesystem.h"
#include "vault.h"

#include "esp_ota_ops.h"

#include "jolt_lib.h"

/* Dbg to test muzzle */
#include "hal/radio/radio.h"

#if CONFIG_HEAP_TRACING
#include "esp_heap_trace.h"
#define HEAP_TRACING_NUM_RECORDS 100
static heap_trace_record_t trace_records[HEAP_TRACING_NUM_RECORDS];
#endif

const jolt_version_t JOLT_VERSION = {
    .major = 0,
    .minor = 1,
    .patch = 0,
    .release = JOLT_VERSION_DEV,
};

static const char TAG[] = "main";

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
    jolt_gui_indev_init();

    /* These lines are for configuring the ADC for sensing battery voltage;
     * refactor these to be somewhere else later */
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(JOLT_ADC1_VBATT, ADC_ATTEN_DB_11);

    /* Run Key/Value Storage Initialization */
    storage_startup();

    // ==== Initialize the file system ====
    jolt_fs_init();

    /* Create GUI */
    {
        ESP_LOGI(TAG, "Creating GUI");
        jolt_lang_t lang;
        storage_get_u8(&lang, "user", "lang", CONFIG_JOLT_LANG_DEFAULT );
        jolt_lang_set( lang ); // Internally initializes the theme
    }
    {
        BaseType_t ret;
        ESP_LOGI(TAG, "Creating LVGL Draw Task");
        ret = xTaskCreate(littlevgl_task,
                    "LVGL_Draw", CONFIG_JOLT_TASK_STACK_SIZE_LVGL,
                    NULL, CONFIG_JOLT_TASK_PRIORITY_LVGL, NULL);
        if( pdPASS != ret ){
            if( errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY == ret ) {
                ESP_LOGE(TAG, "%s Couldn't allocate memory for LVGL Drawing Task",
                        __func__);
            }
            else {
                ESP_LOGE(TAG, "%s Failed to start drawing task, error_code=%d",
                        __func__, ret);
            }
        }
    }

    /* Create StatusBar */
    statusbar_create();

    /* Initialize Wireless */
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    esp_log_level_set("wifi", ESP_LOG_NONE);
    set_jolt_cast();
    /* todo; double check the quality of RNG sources with wifi off */
    {
        uint8_t wifi_en;
        storage_get_u8(&wifi_en, "user", "wifi_en", 0 );
        if( wifi_en ) {
            jolt_wifi_start();
        }
    }

    {
        bool first_boot = ( false == vault_setup() );
        if( first_boot ) {
            /* Create First Boot Screen */
            jolt_gui_first_boot_create();
        }
        else{
            /* Create Home Menu */
            jolt_gui_menu_home_create();
        }
    }

    ESP_LOGI(TAG, "Starting Hardware Monitors");
    xTaskCreate(jolt_hw_monitor_task,
            "HW_Monitor", CONFIG_JOLT_TASK_STACK_SIZE_HW_MONITORS,
            NULL, CONFIG_JOLT_TASK_PRIORITY_HW_MONITORS, NULL);

    jolt_led_setup();

    // Initiate Console
    {
        uint8_t bluetooth_en;
        storage_get_u8(&bluetooth_en, "user", "bluetooth_en", 0 );
        if( bluetooth_en ) {
            jolt_bluetooth_start();
        }
    }

    console_init();
    console_start(); // starts a task adding uart commands to the command queue. Also starts the task to process the command queue.

    /* Setup Power Management */
#if false && CONFIG_PM_ENABLE
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
        esp_pm_config_esp32_t cfg = {
            .max_freq_mhz = CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ,
            .min_freq_mhz = 40,
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
            .light_sleep_enable = true
#endif
        };
        ESP_ERROR_CHECK(esp_pm_configure(&cfg));
    }
#endif

#if 0
    /* radio muzzling debugging */
    vTaskDelay(pdMS_TO_TICKS(10000));
    JOLT_RADIO_OFF_CTX{
        vTaskDelay(pdMS_TO_TICKS(700));
    }
#endif

}
#endif
