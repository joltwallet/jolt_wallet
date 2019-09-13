/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "sdkconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_err.h"
#include "esp_log.h"
#include "bootloader_random.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "esp_event.h"
#include "esp_pm.h"
#include "test_utils.h"
#include "unity.h"
#include "esp_task_wdt.h"

#include <driver/adc.h>
#include "esp_adc_cal.h"


#include "lv_conf.h"
#include "lvgl/lvgl.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/menus/home.h"

#include "hal/radio/bluetooth.h"
#include "hal/radio/wifi.h"
#include "hal/display.h"
#include "hal/i2c.h"
#include "hal/storage/storage.h"
#include "hal/hw_monitor.h"
#include "hal/led.h"
#include "syscore/filesystem.h"
#include "syscore/cli.h"
#include "syscore/cli_ble.h"
#include "syscore/cli_uart.h"
#include "syscore/partition.h"
#include "syscore/noise.h"
#include "vault.h"

#include "esp_ota_ops.h"

#include "jolt_lib.h"

/* Dbg to test muzzle */
#include "hal/radio/radio.h"

#if CONFIG_HEAP_TASK_TRACING
#include "esp_heap_trace.h"
#define HEAP_TRACING_NUM_RECORDS 100
static heap_trace_record_t trace_records[HEAP_TRACING_NUM_RECORDS];
#endif /* CONFIG_HEAP_TRACING */

/* DO NOT directly change JOLT_OS_VERSION. It is updated via bumpversion */
const jolt_version_t JOLT_OS_VERSION = { .major = 0, .minor = 0, .patch = 0 };

/* todo: this information should be stored in another partition */
const jolt_version_t JOLT_HW_VERSION = {
    .major = JOLT_HW_JOLT,
    .minor = 1,
    .patch = 0,
};

static const char TAG[] = "main";

#if UNIT_TESTING
static void unity_task(void *pvParameters){
    vTaskDelay(2); /* Delay a bit to let the main task be deleted */
    esp_task_wdt_delete( xTaskGetIdleTaskHandle() );
    unity_run_menu(); /* Doesn't return */
}

void app_main(void)
{
    ESP_LOGI(TAG, "Running Unit Tester");
    xTaskCreatePinnedToCore(unity_task, "unityTask", UNITY_FREERTOS_STACK_SIZE, NULL,
                            UNITY_FREERTOS_PRIORITY, NULL, UNITY_FREERTOS_CPU);
}
#else

static void littlevgl_task() {
    bool cancel_rollback = false;
    ESP_LOGI(TAG, "Starting draw loop");
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for( uint8_t counter=0;; vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(10) ), counter++ ) {
        if( counter == 20 && cancel_rollback == false ) {
            ESP_LOGI(TAG, "Marking app as valid.");
            ESP_ERROR_CHECK( esp_ota_mark_app_valid_cancel_rollback() );
            cancel_rollback = true;
        }
        JOLT_GUI_CTX{
            lv_task_handler();
        }
    }
    ESP_LOGE(TAG, "Draw Loop Exitted"); // Should never reach here
    abort();
}


void app_main(void) {
    /* Setup Heap Logging */
    #if CONFIG_HEAP_TRACING
    {
        ESP_ERROR_CHECK( heap_trace_init_standalone(trace_records,
                    HEAP_TRACING_NUM_RECORDS) );
    }
    #endif

    /* Verify Partitions */
    {
#if CONFIG_JOLT_TABLE_CHECK
        jolt_partition_check_table();
#endif
#if CONFIG_JOLT_BOOTLOADER_CHECK
        jolt_partition_check_bootloader();
#endif

        /* Check currently running partition */
        {
            const esp_partition_t *partition = esp_ota_get_running_partition();
            ESP_LOGI(TAG, "Currently Running %s at 0x%08X.",
                    partition->label, partition->address);
            ESP_LOGI(TAG, "Partition is %sencrypted.",
                    partition->encrypted ? EMPTY_STR : "NOT ");
        }
    }

#if CONFIG_EFUSE_VIRTUAL 
    ESP_LOGW(TAG, 
            "********************************************\n"
            "* WARNING: EMULATING ALL EFUSE ACTIONS.    *\n"
            "* -- DO NOT EMULATE ON CONSUMER RELEASES --*\n"
            "********************************************\n"
            );

#endif

#if CONFIG_BOOTLOADER_EFUSE_SECURE_VERSION_EMULATE
    ESP_LOGW(TAG, 
            "********************************************\n"
            "* WARNING: EMULATING EFUSE SECURE VERSION. *\n"
            "* -- DO NOT EMULATE ON CONSUMER RELEASES --*\n"
            "********************************************\n"
            );
#endif


    /* Ensure High Quality RNG */
#if CONFIG_NO_BLOBS
    bootloader_random_enable();
#endif

    /* Setup and Install I2C Driver */
    {
        if( ESP_OK != i2c_driver_setup() ) {
            ESP_LOGE(TAG, "Failed to install i2c driver");
            abort();
        }
    }

    /* Run Key/Value Storage Initialization */
    {
        ESP_LOGI(TAG, "Initializing Storage");
        /* May require good RNG if setting up ATAES132A for the first time */
        if( !storage_startup() ){
            ESP_LOGE(TAG, "Failed to initialize storage.");
            abort();
        }
    }

    /* Create Default System Event Loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Initialize WiFi */
    {
        esp_log_level_set("wifi", ESP_LOG_NONE);
        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                    ESP_EVENT_ANY_ID, &jolt_wifi_event_handler, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                    IP_EVENT_STA_GOT_IP, &jolt_wifi_event_handler, NULL));

        ESP_ERROR_CHECK( jolt_network_client_init_from_nvs() );

        if( jolt_wifi_get_en() ) {
            ESP_LOGI(TAG, "Starting WiFi");
            jolt_wifi_start();
        }
    }

    /* Initialize Bluetooth */
    {
        esp_vfs_dev_ble_spp_register();
        uint8_t bluetooth_en;
#if CONFIG_JOLT_BT_ENABLE_DEFAULT
        storage_get_u8(&bluetooth_en, "user", "bluetooth_en", 1);
#else
        storage_get_u8(&bluetooth_en, "user", "bluetooth_en", 0);
#endif
        if( 1 == bluetooth_en ) {
            ESP_LOGI(TAG, "Starting Bluetooth");
            jolt_bluetooth_start();
        }
#if CONFIG_JOLT_BT_DEBUG_ALWAYS_ADV
        jolt_bluetooth_adv_all_start();
#endif
    }

    /* Start side-channel mitigation noise */
    jolt_noise_init();

    /* Initialize LVGL graphics system */
    {
        ESP_LOGI(TAG, "Initializing LVGL graphics system");
        lv_init();
        display_init();

        /* Set GUI Language 
         * This must be done before setting up indev so that the group 
         * style editors can be properly initialized from the theme. */
        {
            ESP_LOGI(TAG, "Creating GUI");
            jolt_lang_t lang;
            storage_get_u8( &lang, "user", "lang", CONFIG_JOLT_LANG_DEFAULT );
            jolt_lang_set( lang ); // Internally initializes the theme
        }

        jolt_gui_indev_init();
    }

    /* Configure VBATT reading ADC */
    {
        adc1_config_width( ADC_WIDTH_BIT_12 );
        adc1_config_channel_atten( JOLT_ADC1_VBATT, ADC_ATTEN_DB_11 );
    }

    /* Initialize the file system */
    {
        ESP_LOGI(TAG, "Initializing Filesystem");
        jolt_fs_init();
    }

    /* Create Hardware Monitors */
    {
        ESP_LOGI(TAG, "Starting Hardware Monitors");
        xTaskCreate(jolt_hw_monitor_task,
                "HW_Monitor", CONFIG_JOLT_TASK_STACK_SIZE_HW_MONITORS,
                NULL, CONFIG_JOLT_TASK_PRIORITY_HW_MONITORS, NULL);
    }

    /* Create GUI Drawing Loop */
    {
        BaseType_t ret;
        ESP_LOGI(TAG, "Creating LVGL Draw Task");
        ret = xTaskCreatePinnedToCore(littlevgl_task,
                    "LVGL_Draw", CONFIG_JOLT_TASK_STACK_SIZE_LVGL,
                    NULL, CONFIG_JOLT_TASK_PRIORITY_LVGL, NULL, portNUM_PROCESSORS-1);
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

    /* Create GUI StatusBar */
    {
        ESP_LOGI(TAG, "Creating Statusbar");
        statusbar_create();
    }

    /* Check and run first-boot routine if necessary */
    {
        ESP_LOGI(TAG, "Setting up Vault");
        if( vault_setup() ) {
            /* Create Home Menu */
            jolt_gui_menu_home_create();
        }
        else {
            /* Create First Boot Screen */
            jolt_gui_first_boot_create();
        }
    }

    /* Capacitive Touch LED Setup */
    {
        jolt_led_setup();
    }

    /* Initialize Console */
    {
        ESP_LOGI(TAG, "Initializing Console");
        jolt_cli_init();
        jolt_cli_uart_init();
        jolt_cli_ble_init();
    }

    /* Setup Power Management */
#if CONFIG_PM_ENABLE
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
        esp_pm_config_esp32_t cfg = {
            .max_freq_mhz = CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ,
            .min_freq_mhz = 40,
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
            .light_sleep_enable = true
#endif /* CONFIG_FREERTOS_USE_TICKLESS_IDLE */
        };
        ESP_ERROR_CHECK( esp_pm_configure(&cfg) );
    }
#endif /* CONFIG_PM_ENABLE */
}

#endif /* UNIT_TESTING */
