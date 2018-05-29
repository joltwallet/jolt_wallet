#include "menu8g2.h"
#include "vault.h"
#include "globals.h"
#include "loading.h"
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

static const char* TAG = "console";

volatile TaskHandle_t console_h = NULL;


static int free_mem(int argc, char** argv) {
    printf("Free: %d bytes\n", esp_get_free_heap_size());
    return 0;
}

static int nano_count(int argc, char** argv) {
    printf("nano_count\n");
    uint32_t block_count = nanoparse_lws_block_count();
    printf("Returned: %d\n", block_count);
    return 0;
}

static int nano_balance(int argc, char** argv) {
    size_t disp_buffer_size = 8 * u8g2_GetBufferTileHeight(&u8g2) *
            u8g2_GetBufferTileWidth(&u8g2);
    uint8_t *old_disp_buffer = malloc(disp_buffer_size);
    memcpy(old_disp_buffer, u8g2_GetBufferPtr(&u8g2), disp_buffer_size);

    vault_rpc_t rpc;
    double display_amount;
    
    /******************
     * Get My Address *
     ******************/
    nvs_handle nvs_h;
    init_nvm_namespace(&nvs_h, "nano");
    if(ESP_OK != nvs_get_u32(nvs_h, "index", &(rpc.nano_public_key.index))){
        rpc.nano_public_key.index = 0;
    }
    nvs_close(nvs_h);
    
    rpc.type = NANO_PUBLIC_KEY;
    if(vault_rpc(&rpc) != RPC_SUCCESS){
        printf("User cancelled.\n");
        display_amount = -1;
        goto exit;
    }
    char address[ADDRESS_BUF_LEN];
    nl_public_to_address(address, sizeof(address),
            rpc.nano_public_key.block.account);
    printf("Address: %s\n", address);
    
    /********************************************
     * Get My Account's Frontier Block *
     ********************************************/
    // Assumes State Blocks Only
    // Outcome:
    //     * frontier_hash, frontier_block
    nl_block_t frontier_block;
    nl_block_init(&frontier_block);
    memcpy(frontier_block.account, rpc.nano_public_key.block.account, BIN_256);
    
    switch( nanoparse_lws_frontier_block(&frontier_block) ){
        case E_SUCCESS:
            if( E_SUCCESS != nl_mpi_to_nano_double(&(frontier_block.balance),
                                                   &display_amount) ){
                display_amount = -1;
            }
            break;
        default:
            display_amount = 0;
            break;
    }

    if( display_amount >= 0 ){
        printf("Balance: %0.4lf Nano\n", display_amount);
    }
    else{
        printf("Error\n");
    }

    exit:
        memcpy(u8g2_GetBufferPtr(&u8g2), old_disp_buffer, disp_buffer_size);
        free(old_disp_buffer);
        return display_amount;
}

void console_task() {
    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    esp_log_level_set("*", ESP_LOG_NONE);
    const char* prompt = "esp32> ";

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
                "ConsoleTask", 32000,
                NULL, 15,
                (TaskHandle_t *) &console_h);
    return  &console_h;
}

void menu_console(menu8g2_t *prev){
    /* On-Device GUI for Starting Console */
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);

    if(console_h){
        menu8g2_display_text_title(&menu,
                "Console is alread running.",
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
        .command = "nano_count",
        .help = "Get the current Nano block count",
        .hint = NULL,
        .func = &nano_count,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "nano_balance",
        .help = "Get the current Nano Balance",
        .hint = NULL,
        .func = &nano_balance,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "free",
        .help = "Get the total size of heap memory available",
        .hint = NULL,
        .func = &free_mem,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
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
                                         256, 0, 0, NULL, 0) );
    
    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_CONSOLE_UART_NUM);
    
    /* Initialize the console */
    esp_console_config_t console_config = {
        .max_cmdline_args = 8,
        .max_cmdline_length = 256,
    };
    ESP_ERROR_CHECK( esp_console_init(&console_config) );
    
    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);
    
    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);
    
    /* Set command history size */
    linenoiseHistorySetMaxLen(20);

    /* Register commands */
    console_register_commands();
}

