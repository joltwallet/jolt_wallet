#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "string.h"

#include "hal/radio/bluetooth.h"
#include "syscore/cli_ble.h"

#include "driver/uart.h"


#if CONFIG_BT_ENABLED
static void jolt_cli_ble_listener_task( void *param );
static TaskHandle_t listener_task_h = NULL;
#endif

void jolt_cli_ble_init(){
#if CONFIG_BT_ENABLED
    static TaskHandle_t ble_in_task = NULL;
    if( NULL == ble_in_task) {
        xTaskCreate(&jolt_cli_ble_listener_task, "ble_spp_listener", 
                CONFIG_JOLT_TASK_STACK_SIZE_BLE_CONSOLE, NULL,
                CONFIG_JOLT_TASK_PRIORITY_BLE_CONSOLE, &listener_task_h);
    }
#endif
}

#if CONFIG_BT_ENABLED

/**
 * @brief FreeRTOS task that forwards ble_stdin to the CLI engine 
 */
static void jolt_cli_ble_listener_task( void *param ) {
    esp_vfs_dev_ble_spp_register();
    ble_stdin  = fopen("/dev/ble/0", "r");
    ble_stdout = fopen("/dev/ble/0", "w");
    ble_stderr = fopen("/dev/ble/0", "w");

    char *buf = NULL;
    for(;;){
        if ( NULL == buf ) {
            buf = calloc(1, CONFIG_JOLT_CONSOLE_MAX_CMD_LEN);
        }
        char *ptr = buf;
        uint16_t i;
        for(i=0; i<CONFIG_JOLT_CONSOLE_MAX_CMD_LEN-1; i++, ptr++){
            fread(ptr, 1, 1, ble_stdin);
            if('\n' == *ptr){
                *ptr = '\0';
                break;
            }
        }
        if(i>0){
#if LOG_LOCAL_LEVEL >= 4 /* debug */
            {
                const char buf[] = "sending command from ble\n";
                uart_write_bytes(UART_NUM_0, buf, strlen(buf));
            }
#endif
            bool suspend = false;
            if( 0 == strcmp(buf, "upload_firmware") || 0 == strcmp(buf, "upload") ){
                suspend = true;
            }

            jolt_cli_src_t src;
            src.line = buf;
            src.in = ble_stdin;
            src.out = ble_stdout;
            src.err = ble_stderr;
            jolt_cli_set_src( &src );

            buf = NULL;

            if( suspend == true ) jolt_cli_ble_suspend();
        }
    }

    vTaskDelete(NULL);
}
#endif

void jolt_cli_ble_suspend(){
#if CONFIG_BT_ENABLED
    vTaskSuspend( listener_task_h );
#endif
}

void jolt_cli_ble_resume(){
#if CONFIG_BT_ENABLED
    vTaskResume( listener_task_h );
#endif
}
