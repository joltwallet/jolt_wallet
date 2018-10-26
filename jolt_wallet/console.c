/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "sodium.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_console.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "linenoise/linenoise.h"

#include "globals.h"
#include "console.h"
#include "vault.h"
#include "jolt_helpers.h"

#include "jolt_gui/jolt_gui.h"

#include "syscore/console.h"
#include "syscore/launcher.h"

static const char* TAG = "console";
static const char* TITLE = "Console";

volatile TaskHandle_t console_h = NULL;


void console_task() {
    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    const char* prompt = "jolt> ";

    #if CONFIG_JOLT_CONSOLE_OVERRIDE_LOGGING
    esp_log_level_set("*", ESP_LOG_NONE);
    #endif

    printf("\n"
           "Welcome to the Jolt Console.\n"
           "Type 'help' to get the list of commands.\n"
           "\n");

    /* Figure out if the terminal supports escape sequences */
    int probe_status = linenoiseProbe();
    if (probe_status) { /* zero indicates success */
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Putty instead.\n");
        linenoiseSetDumbMode(1);
    }
    
    /* Main loop */
    for(;;) {
        /* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
        char* line = linenoise(prompt);
        if (line == NULL) { /* Ignore empty lines */
            continue;
        }
        if (strcmp(line, "exit") == 0){
            printf("Exiting Console\n");
            break;
        }
        /* Add the command to the history */
        linenoiseHistoryAdd(line);
        
        /* Try to run the command */
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND) {
            // The command could be an app to run console commands from
            char *argv[CONFIG_JOLT_CONSOLE_MAX_ARGS + 1];
            // split_argv modifies line with NULL-terminators
            size_t argc = esp_console_split_argv(line, argv, sizeof(argv));
            if( launch_file(argv[0], "console", argc-1, argv+1) ) {
                printf("Unsuccessful command\n");
            }
        } else if (err == ESP_ERR_INVALID_ARG) {
            // command was empty
        } else if (err == ESP_OK && ret != ESP_OK) {
            printf("Command returned non-zero error code: 0x%x\n", ret);
        } else if (err != ESP_OK) {
            printf("Internal error: 0x%x\n", err);
        }
        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }
    
    #if CONFIG_JOLT_CONSOLE_OVERRIDE_LOGGING
    esp_log_level_set("*", CONFIG_LOG_DEFAULT_LEVEL);
    #endif

    console_h = NULL;
    vTaskDelete( NULL );
}

volatile TaskHandle_t *start_console(){
    xTaskCreate(console_task,
                "ConsoleTask", 28000,
                NULL, 19,
                (TaskHandle_t *) &console_h);
    return  &console_h;
}

void menu_console(lv_obj_t * list_btn){
    /* On-Device GUI for Starting Console */

    if(console_h){
        ESP_LOGI(TAG, "Console already running.");
        jolt_gui_scr_text_create(TITLE, "Console is already running.");
    }
    else{
        ESP_LOGI(TAG, "Starting console.");
        start_console();
        jolt_gui_scr_text_create(TITLE, "Console Started.");
    }
}

static void console_register_commands(){

    esp_console_register_help_command();
    
    console_syscore_register();

    /* Register app names */
    // TODO

    /* Register Coin Specific Commands */
    // TODO deprecate this
}

void initialize_console() {
    /* Disable buffering on stdin and stdout */
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    
    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);
    
    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(CONFIG_CONSOLE_UART_NUM,
            CONFIG_JOLT_CONSOLE_UART_RX_BUF_LEN,
            CONFIG_JOLT_CONSOLE_UART_TX_BUF_LEN,
            0, NULL, 0) );
    
    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_CONSOLE_UART_NUM);
    
    /* Initialize the console */
    esp_console_config_t console_config = {
        .max_cmdline_args = CONFIG_JOLT_CONSOLE_MAX_ARGS,
        .max_cmdline_length = CONFIG_JOLT_CONSOLE_MAX_CMD_LEN,
    };
    ESP_ERROR_CHECK( esp_console_init(&console_config) );
    
    /* Configure linenoise line completion library */
    /* Enable multiline editing. */
    linenoiseSetMultiLine(1);

    /* Clear the screen */
#if( JOLT_CONFIG_CONSOLE_STARTUP_CLEAR )
        linenoiseClearScreen();
#endif
    
    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);
    
    /* Set command history size */
    linenoiseHistorySetMaxLen(CONFIG_JOLT_CONSOLE_HISTORY);

    /* Register commands */
    console_register_commands();
}

bool console_check_range_argc(uint8_t argc, uint8_t min, uint8_t max){
    if ( argc > max) {
        printf("Too many input arguments; max %d args, got %d\n",
                max, argc);
        return false;
    }
    if ( argc < min) {
        printf("Too few input arguments; min %d args, got %d\n",
                min, argc);
        return false;
    }

    return true;
}

bool console_check_equal_argc(uint8_t argc, uint8_t expected){
    if ( argc != expected) {
        printf("Incorrect number of input arguments; expected %d args, got %d\n",
                expected, argc);
        return false;
    }
    return true;
}

subconsole_t *subconsole_cmd_init() {
    subconsole_t *subconsole;
    subconsole = calloc(1, sizeof(subconsole_t));
    return subconsole;
}

int subconsole_cmd_register(subconsole_t *subconsole, esp_console_cmd_t *cmd) {
    subconsole_t *new;
    subconsole_t *current = subconsole;
    if( NULL != current->cmd.func ) {
        new = subconsole_cmd_init();
        if( NULL == new ) {
            return -1;
        }
        for(;;){
            if( NULL == current->next ){
                break;
            }
            current = current->next;
        }
        current->next = new;
    }
    else{
        new = current;
    }
    memcpy(&new->cmd, cmd, sizeof(esp_console_cmd_t));
    return 0;
}

static void subconsole_cmd_help(subconsole_t *subconsole) {
    subconsole_t *current = subconsole;
    printf("Help:\n");
    while( current ) {
        printf("    %s - %s\n", current->cmd.command, current->cmd.help);
        current = current->next;
    }
}

int subconsole_cmd_run(subconsole_t *subconsole, uint8_t argc, char **argv) {
    subconsole_t *current = subconsole;
    if( argc > 0 ) {
        while( current ) {
            if( 0 == strcmp(argv[0], current->cmd.command) ) {
                return (current->cmd.func)(argc, argv);
            }
            else if( 0 == strcmp(argv[0], "help") ) {
                subconsole_cmd_help(subconsole);
                return 0;
            }
            current = current->next;
        }
    }
    subconsole_cmd_help(subconsole);
    return -100;
}

void subconsole_cmd_free(subconsole_t *subconsole) {
    subconsole_t *current = subconsole;
    while( current ) {
        subconsole_t *next = current->next;
        free(current);
        current = next;
    }
}
