#include "menu8g2.h"
#include "sodium.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_console.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "linenoise/linenoise.h"

#include <libwebsockets.h>
#include "nano_lws.h"
#include "nano_parse.h"
#include "globals.h"
#include "console.h"
#include "vault.h"
#include "gui.h"
#include "helpers.h"
#include "loading.h"
#include "statusbar.h"
#include "confirmation.h"

#include "coins/nano/console.h"

static const char* TAG = "console";

volatile TaskHandle_t console_h = NULL;


static int free_mem(int argc, char** argv) {
    printf("Free: %d bytes\n", esp_get_free_heap_size());
    return 0;
}

static int task_status(int argc, char** argv) {
    /* Get Task Memory Usage */
    char pcWriteBuffer[1024];
    vTaskList( pcWriteBuffer );
    printf("B - Blocked | R - Ready | D - Deleted | S - Suspended\n"
           "Task            Status Priority High    Task #\n"
           "**********************************************\n");
    printf( pcWriteBuffer );
    return 0;
}

static int cpu_status(int argc, char** argv) {
    /* Gets Task CPU usage statistics */
    char pcWriteBuffer[1024];
    printf("Task            Abs Time (uS)           %%Time\n"
           "*********************************************\n");
    vTaskGetRunTimeStats( pcWriteBuffer );
    printf( pcWriteBuffer );
    return 0;
}

static int wifi_update(int argc, char** argv) {
    int return_code = 0;
    vault_rpc_t rpc;
    SCREEN_SAVE;
    rpc.type = SYSCORE_WIFI_UPDATE;

    if( !console_check_range_argc(argc, 2, 3) ) {
        return_code = 1;
        goto exit;
    }

    if( 3 == argc ) {
        rpc.syscore_wifi_update.pass = argv[2];
    }
    else {
        rpc.syscore_wifi_update.pass = "";
    }

    rpc.syscore_wifi_update.ssid = argv[1];

    vault_rpc_response_t res = vault_rpc(&rpc);
    if( RPC_SUCCESS != res ){
        printf("Error Updating WiFi Settings\n");
        return_code = res;
        goto exit;
    }

    printf("Wifi Settings Updated. Restarting");
    esp_restart();

    exit:
        SCREEN_RESTORE;
        return return_code;
}

static int mnemonic_restore(int argc, char** argv) {
    const char title[] = "Restore";
    const char prompt[] = "Enter Mnemonic Word: ";
    int return_code = 0;
    char *line;
    CONFIDENTIAL char user_words[24][11];
    CONFIDENTIAL uint8_t index[24];
    menu8g2_t menu;
    menu8g2_init(&menu, (u8g2_t *) &u8g2, input_queue, disp_mutex, NULL, statusbar_update);
    SCREEN_SAVE;

    if( !menu_confirm_action(&menu, "Begin mnemonic restore?") ) {
        return_code = -1;
        goto exit;
    }

    // Generate Random Order for user to input mnemonic
    for(uint8_t i=0; i< sizeof(index); i++){
        index[i] = i;
    }
    shuffle_arr(index, sizeof(index));

    loading_enable();
    for(uint8_t i=0; i < sizeof(index); i++){
        uint8_t j = index[i];
        // Humans like 1-indexing
        char buf[10];
        snprintf(buf, sizeof(buf), "Word %d", j + 1);
        loading_text_title(buf, title);

        line = linenoise(prompt);
        if (line == NULL) { /* Ignore empty lines */
            continue;
        }
        if (strcmp(line, "exit_restore") == 0){
            printf("Aborting mnemonic restore\n");
            linenoiseFree(line);
            return_code = 1;
            goto exit;
        }

        strlcpy(user_words[j], line, sizeof(user_words[j]));
        linenoiseFree(line);

        // verify its a word in the word list
        while(-1 == nl_search_wordlist(user_words[j], strlen(user_words[j]))) {
            printf("Invalid word\n");
            line = linenoise(prompt);
            if (line == NULL) { /* Ignore empty lines */
                continue;
            }
            if (strcmp(line, "exit_restore") == 0){
                printf("Aborting mnemonic restore\n");
                linenoiseFree(line);
                return_code = 1;
                goto exit;
            }

            strlcpy(user_words[j], line, sizeof(user_words[j]));
            linenoiseFree(line);
        }
    }
    sodium_memzero(index, sizeof(index));
    loading_disable();

    // Join Mnemonic into single buffer
    CONFIDENTIAL char mnemonic[MNEMONIC_BUF_LEN];
    size_t offset=0;
    for(uint8_t i=0; i < sizeof(index); i++){
        strlcpy(mnemonic + offset, user_words[i], sizeof(mnemonic) - offset);
        offset += strlen(user_words[i]);
        mnemonic[offset++] = ' ';
    }
    mnemonic[offset - 1] = '\0'; //null-terminate, remove last space

    if( !menu_confirm_action(&menu, "Save restored mnemonic and reboot? CAN NOT BE UNDONE.") ) {
        return_code = -1;
        goto exit;
    }

    store_mnemonic_reboot(&menu, mnemonic);

    exit:
        sodium_memzero(index, sizeof(index));
        sodium_memzero(mnemonic, sizeof(mnemonic));
        SCREEN_RESTORE;
        return return_code;
}

void console_task() {
    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    esp_log_level_set("*", ESP_LOG_NONE);
    const char* prompt = "jolt> ";

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
            printf("Unrecognized command\n");
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
    
    esp_log_level_set("*", CONFIG_LOG_DEFAULT_LEVEL);
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

void menu_console(menu8g2_t *prev){
    /* On-Device GUI for Starting Console */
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);

    if(console_h){
        menu8g2_display_text_title(&menu,
                "Console is already running.",
                "Console");
    }
    else{
        start_console();
        menu8g2_display_text_title(&menu,
                "Console Started.",
                "Console");
    }
}

static void console_register_commands(){
    esp_console_cmd_t cmd;

    esp_console_register_help_command();

    cmd = (esp_console_cmd_t) {
        .command = "free",
        .help = "Get the total size of heap memory available",
        .hint = NULL,
        .func = &free_mem,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "task_status",
        .help = "Memory usage of all running tasks.",
        .hint = NULL,
        .func = &task_status,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "top",
        .help = "CPU-Usage of Tasks.",
        .hint = NULL,
        .func = &cpu_status,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "wifi_update",
        .help = "Update WiFi SSID and Pass.",
        .hint = NULL,
        .func = &wifi_update,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "mnemonic_restore",
        .help = "Restore mnemonic seed.",
        .hint = NULL,
        .func = &mnemonic_restore,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    /* Register Coin Specific Commands */
    console_nano_register();
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
    linenoiseClearScreen();
    
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
