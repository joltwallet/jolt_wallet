/*
 * Known bugs:
 *     * suspending the uart listener task while it's in the linenoise function causes a deadlock on UART. Internally,
 * linenoise calls fread which takes the uart_read_lock mutex. Need an elegant way of releasing that mutex if taken on
 * suspend. Current workaround: suspending the CLI in advanced for certain functions.
 */

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "syscore/cli_uart.h"
#include "driver/uart.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_vfs_dev.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "linenoise/linenoise.h"
#include "syscore/cli.h"

/********************
 * STATIC FUNCTIONS *
 ********************/
static void jolt_cli_uart_listener_task( void *param );

/********************
 * STATIC VARIABLES *
 ********************/
static const char TAG[]             = "syscore/cli_uart";
static TaskHandle_t listener_task_h = NULL;
static const char prompt[]          = "jolt> ";

void jolt_cli_uart_init()
{
    xTaskCreate( jolt_cli_uart_listener_task, "uart_listener", CONFIG_JOLT_TASK_STACK_SIZE_UART_CONSOLE, NULL,
                 CONFIG_JOLT_TASK_PRIORITY_UART_CONSOLE, (TaskHandle_t *)&listener_task_h );
}

static void jolt_cli_uart_listener_task( void *param )
{
    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */

#if CONFIG_JOLT_CONSOLE_OVERRIDE_LOGGING
    esp_log_level_set( "*", ESP_LOG_NONE );
#endif

    printf( "\n"
            "Welcome to the Jolt Console.\n"
            "Type 'help' to get the list of commands.\n"
            "\n" );

    /* Figure out if the terminal supports escape sequences */
    int probe_status = linenoiseProbe();
    if( probe_status ) { /* zero indicates success */
        printf( "\n"
                "Your terminal application does not support escape sequences.\n"
                "Line editing and history features are disabled.\n"
                "On Windows, try using Putty instead.\n" );
        linenoiseSetDumbMode( 1 );
    }

    uart_wait_tx_done( UART_NUM_0, portMAX_DELAY );
    printf( prompt );

    /* Main loop */
    for( ;; ) {
        bool suspend = false;
        /* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
        uart_wait_tx_done( UART_NUM_0, portMAX_DELAY );

        jolt_cli_src_t src;
        char *line;

        line = linenoise( prompt );
        if( line == NULL ) { continue; /* Ignore empty lines */ }

        /* Wait until the command is sent back */
        uart_wait_tx_done( UART_NUM_0, portMAX_DELAY );

        ESP_LOGD( TAG, "UART line at %p", line );

        /* Add the command to the history */
        linenoiseHistoryAdd( line );

        if( 0 == strncmp( line, "upload", 6 )
#if JOLT_GUI_TEST_MENU
            || 0 == strcmp( line, "display" )
#endif
        ) {
            suspend = true;
        }
        else if( 0 == strcmp( line, "ping" ) ) {
            fwrite( "pong\n", 1, 5, stdout );
            continue;
        }

        /* Send the command to the command queue */
        src.line   = line;
        src.in     = stdin;
        src.out    = stdout;
        src.err    = stderr;
        src.prompt = prompt;
        jolt_cli_set_src( &src );

        /* NOTE: if suspended, the cli command MUST unsuspend upon
         * completion. */
        if( suspend ) {
            /* Extra safety precaution for firmware upload */
            jolt_cli_uart_suspend();
        }
        taskYIELD();
    }

#if CONFIG_JOLT_CONSOLE_OVERRIDE_LOGGING
    esp_log_level_set( "*", CONFIG_LOG_DEFAULT_LEVEL );
#endif

    listener_task_h = NULL;
    vTaskDelete( NULL );
}

void jolt_cli_uart_suspend() { vTaskSuspend( listener_task_h ); }

void jolt_cli_uart_resume() { vTaskResume( listener_task_h ); }
