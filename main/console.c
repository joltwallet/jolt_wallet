#include "menu8g2.h"
#include "vault.h"
#include "globals.h"
#include "loading.h"
#include "sodium.h"

#include "esp_console.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "linenoise/linenoise.h"

#include <libwebsockets.h>
#include "nano_lws.h"
#include "nano_parse.h"

#include "console.h"

static const char* TAG = "console";

void initialize_console()
{
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
    linenoiseHistorySetMaxLen(100);
    

}

static int free_mem(int argc, char** argv)
{
    printf("%d\n", esp_get_free_heap_size());
    return 0;
}

static void register_free()
{
    const esp_console_cmd_t cmd = {
        .command = "free",
        .help = "Get the total size of heap memory available",
        .hint = NULL,
        .func = &free_mem,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

static int nano_count(int argc, char** argv)
{
    printf("nano_count\n");
    backend_rpc_t rpc;
    rpc.type = NANO_BLOCK_COUNT;
    if(backend_rpc(&rpc) != RPC_CMD_SUCCESS){
        return 0;
    }
    
    printf("Returned: %d\n", rpc.block_count);
    
    return 0;
}

static void register_nano_count()
{
    const esp_console_cmd_t cmd = {
        .command = "nano_count",
        .help = "Get the current Nano block count",
        .hint = NULL,
        .func = &nano_count,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

void menu_console(menu8g2_t *prev){
    
    esp_log_level_set("*", ESP_LOG_NONE);
    
    //initialize_console();

    
    /* Register commands */
    esp_console_register_help_command();
    register_free();
    register_nano_count();
    
    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
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
    while(true) {
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

}

backend_rpc_response_t backend_rpc(backend_rpc_t *rpc){
    /* Sets up rpc queue, blocks until vault responds. */
    backend_rpc_response_t res;
    
#if LOG_LOCAL_LEVEL >= ESP_LOG_INFO
    int32_t id = randombytes_random();
#endif
    
    rpc->response_queue = xQueueCreate( 1, sizeof(res) );
    
    ESP_LOGI(TAG, "Attempting Backend RPC Send %d; ID: %d", rpc->type, id);
    if(xQueueSend( backend_queue, (void *) &rpc, 0)){
        ESP_LOGI(TAG, "Success: Backend RPC Send %d; ID: %d", rpc->type, id);
        ESP_LOGI(TAG, "Awaiting Backend RPC Send %d response; ID: %d", rpc->type, id);
        xQueueReceive(rpc->response_queue, (void *) &res, portMAX_DELAY);
        ESP_LOGI(TAG, "Success: Backend RPC Send %d response %d; ID: %d",
                 rpc->type, res, id);
    }
    else{
        ESP_LOGI(TAG, "Vault RPC Send %d failed; ID: %d", rpc->type, id);
        res = RPC_QUEUE_FULL;
    }
    
    vQueueDelete(rpc->response_queue);
    ESP_LOGI(TAG, "Response Queue Deleted; ID: %d\n", id);
    return res;
}

void backend_task(void *backend_in){
    /* This task should be ran at HIGHEST PRIORITY
     * This task is essentially a daemon that is the only activity that should
     * be accessing the vault. This task will respond to commands that
     * request some task to be complete
     *
     * vault must already be initialized before starting this task
     * */
    
    backend_t *backend_task = (backend_t *)backend_in;
    
    backend_rpc_t *cmd;
    backend_rpc_response_t response;
    
    /* The vault_queue holds a pointer to the vault_rpc_t object declared
     * by the producer task. Results are directly modified on that object.
     * A response status is sent back on the queue in the vault_rpc_t stating
     * the RPC error code. */
    backend_queue = xQueueCreate( CONFIG_JOLT_VAULT_RPC_QUEUE_LEN, sizeof( backend_rpc_t* ) );
    
    for(;;){
        if( xQueueReceive(backend_queue, &cmd,
                          pdMS_TO_TICKS(CONFIG_JOLT_DEFAULT_TIMEOUT_S * 1000)) ){
            
                // Perform RPC command
                /* MASTER RPC SWITCH STATEMENT */
                switch(cmd->type){
                    case(FREE):
                        ESP_LOGI(TAG, "Executing FREE RPC.");
                        printf("%d\n", esp_get_free_heap_size());
                        break;
                    case NANO_BLOCK_COUNT:
                        ESP_LOGI(TAG, "Executing RESPONSE RPC.");
                        cmd->block_count = nanoparse_lws_block_count();
                        response = RPC_CMD_SUCCESS;
                        break;
                    default:
                        response = RPC_UNDEFINED;
                        break;
                }
            // Send back response
            xQueueOverwrite(cmd->response_queue, &response);
        }
    }
}
