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

#include "console.h"
#include "vault.h"
#include "jolt_helpers.h"

#include "jolt_gui/jolt_gui.h"

#include "syscore/bg.h"
#include "syscore/ota.h"
#include "syscore/launcher.h"
#include "syscore/cmd/jolt_cmds.h"

static const char TAG[] = "console";

TaskHandle_t console_h = NULL;

/* Static Function Declaration */
static void console_syscore_register();


/* Executes the command string */
static void jolt_process_cmd_task(jolt_bg_job_t *bg_job){
    jolt_cmd_t *cmd = jolt_bg_get_param(bg_job); 

    FILE *orig_stdin = stdin;
    FILE *orig_stdout = stdout;
    FILE *orig_stderr = stderr;

    int ret = 0;
    if(NULL == cmd->data){
        goto exit;
    }
    if( NULL == cmd->fd_in ) {
        stdin = orig_stdin;
    }
    else {
        stdin = cmd->fd_in;
    }
    if( NULL == cmd->fd_out ) {
        stdout = orig_stdout;
    }
    else {
        stdout = cmd->fd_out;
    }
    if( NULL == cmd->fd_err ) {
        stderr = orig_stderr;
    }
    else {
        stderr = cmd->fd_err;
    }

    /* Try to run the command */
    esp_err_t err = esp_console_run(cmd->data, &ret);
    if (err == ESP_ERR_NOT_FOUND) {
        // The command could be an app to run console commands from
        char *argv[CONFIG_JOLT_CONSOLE_MAX_ARGS + 1];
        // split_argv modifies line with NULL-terminators
        size_t argc = esp_console_split_argv(cmd->data, argv, sizeof(argv));
        ESP_LOGD(TAG, "Not an internal command; looking for app of name %s", argv[0]);
        // todo, parse passphrase argument
        ESP_LOGI(TAG, "argv[1] %s", argv[1]);
        if( launch_file(argv[0], argc-1, &argv[1], "") ) {
            printf("Unsuccessful command\n");
        }
    } else if (err == ESP_ERR_INVALID_ARG) {
        // command was empty
    } else if (err == ESP_OK && ret != ESP_OK) {
        printf("Command returned non-zero error code: %d\n", ret);
    } else if (err != ESP_OK) {
        printf("Internal error: 0x%x\n", err);
    }
    cmd->return_value = ret;

exit:
    /* De-allocate memory */
    jolt_cmd_del(cmd);
}

void jolt_cmd_del(jolt_cmd_t *cmd){
    if( NULL == cmd ){
        return;
    }
    if( NULL != cmd->data ){
        free(cmd->data);
    }
    if( NULL != cmd->complete ){
        xSemaphoreGive(cmd->complete);
        taskYIELD();
    }
    free(cmd);
}

static void console_task() {
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
        /* Add the command to the history */
        linenoiseHistoryAdd(line);

        /* Send the command to the command queue */
        bool block = false;
        const char blocking_prefix[] = "upload";
        if( 0 == strncmp(line, blocking_prefix, strlen(blocking_prefix))
                || 0 == strcmp(line, "mnemonic_restore")
                || 0 == strcmp(line, "cat") ) {
            block = true; // todo: better stream control
        }
        jolt_cmd_process(line, stdin, stdout, stderr, block);
        vTaskDelay(50/portTICK_PERIOD_MS); // give enough time for quick commands to execute
    }
    
    #if CONFIG_JOLT_CONSOLE_OVERRIDE_LOGGING
    esp_log_level_set("*", CONFIG_LOG_DEFAULT_LEVEL);
    #endif

    console_h = NULL;
    vTaskDelete( NULL );
}

int jolt_cmd_process(char *line, FILE *in, FILE *out, FILE *err, bool block) {
    int return_code = -1;
    jolt_cmd_t *cmd = NULL;
    cmd = malloc(sizeof(jolt_cmd_t));
    if(NULL == cmd) {
        printf("Failed to allocate memory for job parameters.\n");
        return_code = -1;
        goto exit;
    }
    if(block){
        cmd->complete = xSemaphoreCreateBinary();
        if(NULL == cmd->complete){
            printf("Failed to allocate memory for job finish semaphore.\n");
            return_code = -2;
            goto exit;
        }
    }
    else {
        cmd->complete = NULL;
    }

    cmd->data = line;
    cmd->return_value = -1;
    cmd->fd_in = in;
    cmd->fd_out = out;
    cmd->fd_err = err;

    /* Send the job now */
    jolt_bg_create(jolt_process_cmd_task, cmd, NULL);

    /* Wait until the process signals completion */
    if(block){
        xSemaphoreTake(cmd->complete, portMAX_DELAY);
        vSemaphoreDelete(cmd->complete);
        return cmd->return_value;
    }
    return_code = 0;
    return return_code;

exit:
    if( NULL != cmd ) {
        free(cmd);
    }
    return return_code;
}

TaskHandle_t *console_start(){
    /* Start the Task that handles the UART Console IO */
    xTaskCreate(console_task,
                "UART_Console", CONFIG_JOLT_TASK_STACK_SIZE_UART_CONSOLE,
                NULL, CONFIG_JOLT_TASK_PRIORITY_UART_CONSOLE,
                (TaskHandle_t *) &console_h);
    return &console_h;
}

static void console_register_commands() {
    esp_console_register_help_command();
    console_syscore_register();
}

void console_init() {
    /* Disable buffering on stdin and stdout */
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    
    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

#if 1
    uart_config_t uart_config = {
			.baud_rate = CONFIG_CONSOLE_UART_BAUDRATE,
			.data_bits = UART_DATA_8_BITS,
			.parity = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .use_ref_tick = true,
	};
	// Configure UART parameters
	ESP_ERROR_CHECK(uart_param_config(CONFIG_CONSOLE_UART_NUM, &uart_config));
#endif
    
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
        if( 0 == strcmp(argv[0], "help") ) {
            subconsole_cmd_help(subconsole);
            return 0;
        }

        while( current ) {
            if( 0 == strcmp(argv[0], current->cmd.command) ) {
                return (current->cmd.func)(argc, argv);
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

static void console_syscore_register() {
    esp_console_cmd_t cmd;

    cmd = (esp_console_cmd_t) {
        .command = "about",
        .help = "Display system information.",
        .hint = NULL,
        .func = &jolt_cmd_about,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "app_key",
        .help = "Sets app public key. WILL ERASE ALL DATA.",
        .hint = NULL,
        .func = &jolt_cmd_app_key,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "bt_whitelist",
        .help = "Print BLE GAP White List",
        .hint = NULL,
        .func = &jolt_cmd_bt_whitelist,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "cat",
        .help = "Print the contents of a file",
        .hint = NULL,
        .func = &jolt_cmd_cat,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "download",
        .help = "Send specified file from Jolt over UART ymodem",
        .hint = NULL,
        .func = &jolt_cmd_download,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "free",
        .help = "Get the total size of heap memory available",
        .hint = NULL,
        .func = &jolt_cmd_free,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "jolt_cast_update",
        .help = "Update jolt_cast URI.",
        .hint = NULL,
        .func = &jolt_cmd_jolt_cast_update,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "ls",
        .help = "List all files in filesystem",
        .hint = NULL,
        .func = &jolt_cmd_ls,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "mnemonic_restore",
        .help = "Restore mnemonic seed.",
        .hint = NULL,
        .func = &jolt_cmd_mnemonic_restore,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "mv",
        .help = "rename file (src, dst)",
        .hint = NULL,
        .func = &jolt_cmd_mv,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "pmtop",
        .help = "print power management statistics",
        .hint = NULL,
        .func = &jolt_cmd_pmtop,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "reboot",
        .help = "Reboot device.",
        .hint = NULL,
        .func = &jolt_cmd_reboot,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "rm",
        .help = "remove file from filesystem",
        .hint = NULL,
        .func = &jolt_cmd_rm,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "run",
        .help = "launch application",
        .hint = NULL,
        .func = &jolt_cmd_run,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "task_status",
        .help = "Memory usage of all running tasks.",
        .hint = NULL,
        .func = &jolt_cmd_task_status,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "top",
        .help = "CPU-Usage of Tasks.",
        .hint = NULL,
        .func = &jolt_cmd_top,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "upload",
        .help = "Enters file UART ymodem upload mode",
        .hint = NULL,
        .func = &jolt_cmd_upload,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "upload_firmware",
        .help = "Update JoltOS",
        .hint = NULL,
        .func = &jolt_cmd_upload_firmware,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "wifi_update",
        .help = "Update WiFi SSID and Pass.",
        .hint = NULL,
        .func = &jolt_cmd_wifi_update,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
