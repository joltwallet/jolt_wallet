//#define LOG_LOCAL_LEVEL 4

#include "syscore/cli_ble.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "hal/radio/bluetooth.h"
#include "string.h"

#if CONFIG_BT_ENABLED
static void jolt_cli_ble_listener_task( void *param );
static TaskHandle_t listener_task_h = NULL;
#endif

void jolt_cli_ble_init()
{
#if CONFIG_BT_ENABLED
    static TaskHandle_t ble_in_task = NULL;
    if( NULL == ble_in_task ) {
        xTaskCreate( &jolt_cli_ble_listener_task, "ble_spp_listener", CONFIG_JOLT_TASK_STACK_SIZE_BLE_CONSOLE, NULL,
                     CONFIG_JOLT_TASK_PRIORITY_BLE_CONSOLE, &listener_task_h );
    }
#endif
}

#if CONFIG_BT_ENABLED

static const char TAG[] = "cli_ble";

/**
 * @brief FreeRTOS task that forwards ble_stdin to the CLI engine
 */
static void jolt_cli_ble_listener_task( void *param )
{
    esp_vfs_dev_ble_spp_register();
    ble_stdin  = fopen( "/dev/ble/0", "r" );
    ble_stdout = fopen( "/dev/ble/0", "w" );
    ble_stderr = fopen( "/dev/ble/0", "w" );

    setvbuf( ble_stdin, NULL, _IONBF, 0 );
    // setvbuf(ble_stdout, NULL, _IONBF, 0);
    setvbuf( ble_stderr, NULL, _IONBF, 0 );

    char *line = NULL;
    for( ;; ) {
        if( NULL == line ) { line = calloc( 1, CONFIG_JOLT_CONSOLE_MAX_CMD_LEN ); }
        char *ptr = line;
        uint16_t i;
        for( i = 0; i < CONFIG_JOLT_CONSOLE_MAX_CMD_LEN - 1; i++, ptr++ ) {
            fread( ptr, 1, 1, ble_stdin );
            if( '\n' == *ptr ) {
                *ptr = '\0';
                break;
            }
        }
        if( i > 0 ) {
            ESP_LOGD( TAG, "sending command from ble: \"%s\"", line );
            bool suspend = false;
            if( 0 == strncmp( line, "upload", 6 ) ) {
                suspend = true;
            }
            else if( 0 == strcmp( line, "ping" ) ) {
                fwrite( "pong\n", 1, 5, ble_stdout );
                continue;
            }

            jolt_cli_src_t src;
            src.line   = line;
            src.in     = ble_stdin;
            src.out    = ble_stdout;
            src.err    = ble_stderr;
            src.prompt = NULL;
            jolt_cli_set_src( &src );

            line = NULL;

            /* NOTE: if suspended, the cli command MUST unsuspend upon
             * completion. */
            if( suspend == true ) {
                jolt_cli_ble_suspend();
            }
        }
    }

    vTaskDelete( NULL );
}
#endif

void jolt_cli_ble_suspend()
{
#if CONFIG_BT_ENABLED
    vTaskSuspend( listener_task_h );
#endif
}

void jolt_cli_ble_resume()
{
#if CONFIG_BT_ENABLED
    vTaskResume( listener_task_h );
#endif
}
