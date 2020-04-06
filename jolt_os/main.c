/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include <driver/adc.h>
#include <esp_system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bootloader_random.h"
#include "esp_adc_cal.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_freertos_hooks.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_pm.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/display.h"
#include "hal/hw_monitor.h"
#include "hal/i2c.h"
#include "hal/led.h"
#include "hal/radio/bluetooth.h"
#include "hal/radio/wifi.h"
#include "hal/storage/storage.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/menus/home.h"
#include "jolt_lib.h"
#include "lv_conf.h"
#include "lvgl/lvgl.h"
#include "sdkconfig.h"
#include "syscore/cli.h"
#include "syscore/cli_ble.h"
#include "syscore/cli_uart.h"
#include "syscore/ext_storage.h"
#include "syscore/filesystem.h"
#include "syscore/noise.h"
#include "syscore/partition.h"
#include "test_utils.h"
#include "unity.h"
#include "vault.h"

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
static void unity_task( void *pvParameters )
{
    vTaskDelay( 2 ); /* Delay a bit to let the main task be deleted */
    esp_task_wdt_delete( xTaskGetIdleTaskHandle() );
    unity_run_menu(); /* Doesn't return */
}
#endif

static void littlevgl_task()
{
    FILE *default_in, *default_out, *default_err;
    default_in  = stdin;
    default_out = stdout;
    default_err = stderr;

    bool cancel_rollback = false;
    ESP_LOGI( TAG, "Starting draw loop" );
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for( uint8_t counter = 0;; vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 10 ) ), counter++ ) {
        if( counter == 20 && cancel_rollback == false ) {
            /* If the code has made it this far, chances are slim of an
             * irrecoverable crash */
            ESP_LOGI( TAG, "Marking app as valid." );
            ESP_ERROR_CHECK( esp_ota_mark_app_valid_cancel_rollback() );
            cancel_rollback = true;
        }
        stdin  = default_in;
        stdout = default_out;
        stderr = default_err;
        jolt_gui_apply_stdstream();

        JOLT_GUI_CTX { lv_task_handler(); }
    }
    ESP_LOGE( TAG, "Draw Loop Exitted" );  // Should never reach here
    abort();
}

static int log_to_uart( const char *format, va_list arg )
{
    char *buf;
    int n;
    n = vasprintf( &buf, format, arg );
    if( -1 == n ) return -1;
    uart_write_bytes( CONFIG_ESP_CONSOLE_UART_NUM, buf, n );
    free( buf );
    return n;
}

void app_main( void )
{
/* Setup Heap Logging */
#if CONFIG_HEAP_TRACING
    {
        ESP_ERROR_CHECK( heap_trace_init_standalone( trace_records, HEAP_TRACING_NUM_RECORDS ) );
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
            ESP_LOGI( TAG, "Currently Running %s at 0x%08X.", partition->label, partition->address );
            ESP_LOGI( TAG, "Partition is %sencrypted.", partition->encrypted ? EMPTY_STR : "NOT " );
        }
    }

#if CONFIG_EFUSE_VIRTUAL
    ESP_LOGW( TAG, "\n"
                   "********************************************\n"
                   "* WARNING: EMULATING ALL EFUSE ACTIONS.    *\n"
                   "* -- DO NOT EMULATE ON CONSUMER RELEASES --*\n"
                   "********************************************\n" );

#endif

#if CONFIG_BOOTLOADER_EFUSE_SECURE_VERSION_EMULATE
    ESP_LOGW( TAG, "\n"
                   "********************************************\n"
                   "* WARNING: EMULATING EFUSE SECURE VERSION. *\n"
                   "* -- DO NOT EMULATE ON CONSUMER RELEASES --*\n"
                   "********************************************\n" );
#endif

#if CONFIG_HEAP_POISONING_DISABLE
    ESP_LOGW( TAG, "\n"
                   "********************************************\n"
                   "* WARNING: HEAP POISONING DISABLED.        *\n"
                   "* -- DO NOT DISABLE ON CONSUMER RELEASES --*\n"
                   "********************************************\n" );
#elif CONFIG_HEAP_POISONING_COMPREHENSIVE
    ESP_LOGW( TAG, "\n"
                   "********************************************\n"
                   "* WARNING: HEAP POISONING COMPREHENSIVE.   *\n"
                   "* PERFORMANCE DEGRADED; ONLY FOR DEBUGGING *\n"
                   "********************************************\n" );
#endif

#if CONFIG_NEWLIB_STDOUT_LINE_ENDING_CRLF
    ESP_LOGW( TAG, "WARNING: UART \\n replaced with \\r\\n" );
#elif CONFIG_NEWLIB_STDOUT_LINE_ENDING_CR
    ESP_LOGW( TAG, "WARNING: UART \\n replaced with \\r" );
#endif

    /* Create Default System Event Loop */
    ESP_ERROR_CHECK( esp_event_loop_create_default() );

    /* Ensure High Quality RNG */
#if CONFIG_NO_BLOBS
    bootloader_random_enable();
#endif

    /* Setup UART */
    {
        /* Disable buffering on stdin and stdout */
        setvbuf( stdin, NULL, _IONBF, 0 );
        setvbuf( stdout, NULL, _IONBF, 0 );

        /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
        esp_vfs_dev_uart_set_rx_line_endings( ESP_LINE_ENDINGS_CR );
        /* Move the caret to the beginning of the next line on '\n' */
        esp_vfs_dev_uart_set_tx_line_endings( ESP_LINE_ENDINGS_CRLF );  // !!!

        uart_config_t uart_config = {
                .baud_rate    = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
                .data_bits    = UART_DATA_8_BITS,
                .parity       = UART_PARITY_DISABLE,
                .stop_bits    = UART_STOP_BITS_1,
                .flow_ctrl    = UART_HW_FLOWCTRL_DISABLE,
                .use_ref_tick = true,
        };
        /* Configure UART parameters */
        ESP_ERROR_CHECK( uart_param_config( CONFIG_ESP_CONSOLE_UART_NUM, &uart_config ) );

        /* Install UART driver for interrupt-driven reads and writes */
        ESP_ERROR_CHECK( uart_driver_install( CONFIG_ESP_CONSOLE_UART_NUM, CONFIG_JOLT_CONSOLE_UART_RX_BUF_LEN,
                                              CONFIG_JOLT_CONSOLE_UART_TX_BUF_LEN, 0, NULL, 0 ) );

        /* Tell VFS to use UART driver */
        esp_vfs_dev_uart_use_driver( CONFIG_ESP_CONSOLE_UART_NUM );

        /* Force logging to always goto UART */
        esp_log_set_vprintf( log_to_uart );
    }

    /* Setup and Install I2C Driver */
    {
        if( ESP_OK != i2c_driver_setup() ) {
            ESP_LOGE( TAG, "Failed to install i2c driver" );
            abort();
        }
    }

    /* Run Key/Value Storage Initialization */
    {
        ESP_LOGI( TAG, "Initializing Storage" );
        /* May require good RNG if setting up ATAES132A for the first time */
        if( !storage_startup() ) {
            ESP_LOGE( TAG, "Failed to initialize storage." );
            abort();
        }
    }

    /* Initialize WiFi */
    {
        // esp_log_level_set( "wifi", ESP_LOG_NONE );

        /* instantiate the app-friendly network client */
        ESP_ERROR_CHECK( jolt_network_client_init_from_nvs() );

        if( jolt_wifi_get_en() ) {
            ESP_LOGI( TAG, "Starting WiFi" );
            jolt_wifi_start();
        }
    }

    /* Initialize Bluetooth */
    {
        esp_vfs_dev_ble_spp_register();
        uint8_t bluetooth_en;
#if CONFIG_JOLT_BT_ENABLE_DEFAULT
        storage_get_u8( &bluetooth_en, "user", "bluetooth_en", 1 );
#else
        storage_get_u8( &bluetooth_en, "user", "bluetooth_en", 0 );
#endif
        if( 1 == bluetooth_en ) {
            ESP_LOGI( TAG, "Starting Bluetooth" );
            jolt_bluetooth_start();
        }
#if CONFIG_JOLT_BT_DEBUG_ALWAYS_ADV
        jolt_bluetooth_adv_all_start();
#endif
    }

    /* Initialize External Storage */
    jolt_ext_storage_init();

    /* Start side-channel mitigation noise */
    jolt_noise_init();

    /* Initialize LVGL graphics system */
    {
        ESP_LOGI( TAG, "Initializing LVGL graphics system" );
        lv_init();
        jolt_display_init();

        /* Set GUI Language
         * This must be done before setting up indev so that the group
         * style editors can be properly initialized from the theme. */
        {
            ESP_LOGI( TAG, "Creating GUI" );
            jolt_lang_t lang;
            storage_get_u8( &lang, "user", "lang", CONFIG_JOLT_LANG_DEFAULT );
            jolt_lang_set( lang );  // Internally initializes the theme
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
        ESP_LOGI( TAG, "Initializing Filesystem" );
        jolt_fs_init();
    }

    /* Create Hardware Monitors */
    {
        ESP_LOGI( TAG, "Starting Hardware Monitors" );
        jolt_hw_monitor_init();
    }

    /* Capacitive Touch LED Setup */
    {
        jolt_led_setup();
    }

    /* Create GUI StatusBar */
    {
        ESP_LOGI( TAG, "Creating Statusbar" );
        jolt_gui_statusbar_create();
        jolt_gui_statusbar_set_icons( JOLT_GUI_STATUSBAR_ICON_BATTERY | JOLT_GUI_STATUSBAR_ICON_CHIP |
                                      JOLT_GUI_STATUSBAR_ICON_BLUETOOTH | JOLT_GUI_STATUSBAR_ICON_WIFI |
                                      JOLT_GUI_STATUSBAR_ICON_LOCK );
    }

    /* Create GUI Drawing Loop */
    {
        BaseType_t ret;
        ESP_LOGI( TAG, "Creating LVGL Draw Task" );
        ret = xTaskCreatePinnedToCore( littlevgl_task, "LVGL_Draw", CONFIG_JOLT_TASK_STACK_SIZE_LVGL, NULL,
                                       CONFIG_JOLT_TASK_PRIORITY_LVGL, NULL, portNUM_PROCESSORS - 1 );
        if( pdPASS != ret ) {
            if( errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY == ret ) {
                ESP_LOGE( TAG, "%s Couldn't allocate memory for LVGL Drawing Task", __func__ );
            }
            else {
                ESP_LOGE( TAG, "%s Failed to start drawing task, error_code=%d", __func__, ret );
            }
        }
    }

    bool first_boot;
    ESP_LOGI( TAG, "Setting up Vault" );
    first_boot = vault_setup();

#if UNIT_TESTING
    /* Unit Testing Console Interface */
    (void)first_boot;
    jolt_gui_statusbar_set_icons( 0 );
    ESP_LOGI( TAG, "Running Unit Tester" );
    xTaskCreatePinnedToCore( unity_task, "unityTask", UNITY_FREERTOS_STACK_SIZE, NULL, UNITY_FREERTOS_PRIORITY, NULL,
                             UNITY_FREERTOS_CPU );
#else
    ESP_LOGI( TAG, "Setting up Vault" );
    if( first_boot )
        jolt_gui_menu_home_create();
    else
        jolt_gui_first_boot_create();

    /* Initialize Console */
    {
        ESP_LOGI( TAG, "Initializing Console" );
        jolt_cli_init();
        jolt_cli_uart_init();
        jolt_cli_ble_init();
    }
#endif

/* Setup Power Management */
#if CONFIG_PM_ENABLE && CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ > 80
    {
        esp_err_t err;
        ESP_LOGI( TAG, "Initializing power management" );
        vTaskDelay( pdMS_TO_TICKS( 5000 ) );
        esp_pm_config_esp32_t cfg = {
            .max_freq_mhz = CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ,
            .min_freq_mhz = 80,
    #if CONFIG_FREERTOS_USE_TICKLESS_IDLE
            .light_sleep_enable = true
    #endif /* CONFIG_FREERTOS_USE_TICKLESS_IDLE */
        };
        err = esp_pm_configure( &cfg );
        if( ESP_OK != err ) {
            ESP_LOGE( TAG, "Error %s initializing power management.", esp_err_to_name( err ) );
            esp_restart();
        }
        ESP_LOGI( TAG, "Power management initialized" );
    }
#endif /* CONFIG_PM_ENABLE */
}
