//#define LOG_LOCAL_LEVEL 4

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_console.h"
#include "linenoise/linenoise.h"

#include "syscore/bg.h"
#include "syscore/cli.h"
#include "syscore/cli_uart.h"
#include "syscore/cli_ble.h"
#include "syscore/ota.h"
#include "syscore/launcher.h"
#include "syscore/cmd/jolt_cmds.h"

/********************
 * STATIC FUNCTIONS *
 ********************/
static void jolt_cli_dispatcher_task( void *param );
static int32_t jolt_cli_process_task(jolt_bg_job_t *bg_job);
static void jolt_cli_cmds_register();
static bool jolt_cli_get_src(jolt_cli_src_t *src, int16_t timeout);

static bool jolt_cli_take_src_lock(TickType_t ticks_to_wait);
static bool jolt_cli_give_src_lock();

/********************
 * STATIC VARIABLES *
 ********************/
static const char TAG[] = "syscore/cli";
static SemaphoreHandle_t src_lock = NULL;        /**< Only allow one source at a time while executing */
static FILE * src_lock_in = NULL;                /**< Pointer to the stdin stream that took the src_lock */
static QueueHandle_t msg_queue = NULL;           /**< Queue that holds char ptrs to data sent over CLI. */
static QueueHandle_t ret_val_queue = NULL;       /**< Queue that the cmd populates when complete. */
static volatile bool app_call = false;


void jolt_cli_init() {
    msg_queue = xQueueCreate(5, sizeof(jolt_cli_src_t));
    if( NULL == msg_queue) goto exit;

    src_lock = xSemaphoreCreateBinary();
    if( NULL == src_lock ) goto exit;
    xSemaphoreGive(src_lock);

    ret_val_queue = xQueueCreate(1, sizeof(int));
    if( NULL == ret_val_queue) goto exit;

    BaseType_t res;
    res = xTaskCreate( jolt_cli_dispatcher_task, "cli_dispatcher",
            2048, NULL, 6, NULL);
    if( pdPASS != res ) goto exit;

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

    jolt_cli_cmds_register();

    return;
exit:
    esp_restart();
}

char *jolt_cli_get_line(int16_t timeout){
    jolt_cli_src_t src = { 0 };

    if( !jolt_cli_get_src(&src, timeout) ) return NULL;

    stdin = src.in;
    stdout = src.out;
    stderr = src.err;

    return src.line;
}

/**
 * @brief Gets a jolt_cli_src_t from the queue.
 * @param[out] src
 * @param[in] timeout
 * @return True if an item is being successfully returned.
 */
static bool jolt_cli_get_src(jolt_cli_src_t *src, int16_t timeout){
    TickType_t delay;
    bool res;

    /* Convert timeout into ticks */
    delay = timeout < 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeout);

    res = (pdTRUE == xQueueReceive(msg_queue, src, delay));

    return res;
}

/**
 * @brief Obtain the `src_lock` semaphore.
 * @param[in] ticks_to_wait FreeRTOS ticks to wait before timing out
 * @return True if successfully obtained. False otherwise.
 */
static bool jolt_cli_take_src_lock(TickType_t ticks_to_wait){
    bool res;
    ESP_LOGD(TAG, "Taking src_lock;");
    res = xSemaphoreTake(src_lock, ticks_to_wait);
    if(res) ESP_LOGD(TAG, "Took src_lock");
    else ESP_LOGD(TAG, "Failed to take src_lock");
    return res;
}

static bool jolt_cli_give_src_lock(){
    bool res;
    ESP_LOGD(TAG, "Giving src_lock;");
    res = xSemaphoreGive( src_lock );
    ESP_LOGD(TAG, "Gave src_lock");
    return res;
}

void jolt_cli_set_src(jolt_cli_src_t *src) {
    /* Only allow a queue of commands from a single input source at a time */
    portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&myMutex);
    if( jolt_cli_take_src_lock(0) ) {
        /* Successfully taken */
        src_lock_in = src->in;
        xQueueSend(msg_queue, src, 0);
        portEXIT_CRITICAL(&myMutex);
    }
    else {
        if( src->in == src_lock_in ){
            xQueueSend(msg_queue, src, 0);
            portEXIT_CRITICAL(&myMutex);
        }
        else {
            portEXIT_CRITICAL(&myMutex);
            ESP_LOGD(TAG, "took src_lock");
            src_lock_in = src->in;
            xQueueSend(msg_queue, src, 0);
        }
    }
}

void jolt_cli_suspend(){
    /* Pause all CLI Tasks */
    jolt_cli_uart_suspend();
    jolt_cli_ble_suspend();
}

void jolt_cli_resume(){
    /* Resume all CLI Tasks */
    jolt_cli_uart_resume();
    jolt_cli_ble_resume();
}

void jolt_cli_return( int val ) {
    xQueueSend(ret_val_queue, &val, portMAX_DELAY);
    if( JOLT_CLI_NON_BLOCKING != val ) {
        if( 0 != val ) {
            printf("Command returned non-zero error code: %d\n", val);
        }
        if( app_call ) {
            launch_dec_ref_ctr();
            app_call = false;
        }
    }
}

/**
 * @brief FreeRTOS task function that dequeues CLI lines and dispatches them
 * to the BG task for execution
 */
static void jolt_cli_dispatcher_task( void *param ) {
    for(;;){
        int ret_val;
        jolt_cli_src_t src = { 0 };

        /* Get CLI info off of queue */
        jolt_cli_get_src(&src, -1);

        /* Dispatch job to bg */
        ESP_LOGD(TAG, "Dispatching \"%s\" to bg", src.line);
        jolt_bg_create(jolt_cli_process_task, &src, NULL);

        /* Block until job finished. */
        do {
            xQueueReceive(ret_val_queue, &ret_val, portMAX_DELAY);
        } while(JOLT_CLI_NON_BLOCKING == ret_val);

        /* Release source mutex */
        jolt_cli_give_src_lock();

        /* Reprompt the user */
        if( NULL != src.prompt ){
            printf("\n");
            printf(src.prompt);
        }

        ESP_LOGD(TAG, "Freeing src.line");
        free( src.line );
    }
}

/**
 * @brief Executes the command string in the bg task.
 * @return Time in mS to wait before repeating.
 */
static int32_t jolt_cli_process_task(jolt_bg_job_t *bg_job) {
    jolt_cli_src_t *src = jolt_bg_get_param(bg_job); 
    int ret = 0;
    esp_err_t err;

    if(NULL == src->line) goto exit;

    if( src->in ) stdin = src->in;
    if( src->out ) stdout = src->out;
    if( src->err ) stderr = src->err;

    /* Try to run the command */
    err = esp_console_run(src->line, &ret);
    switch( err ) {
        case ESP_OK:
            jolt_cli_return(ret);
            break;
        case ESP_ERR_NOT_FOUND: {
            /* The command could be an app to run console commands from */
            static char *argv[CONFIG_JOLT_CONSOLE_MAX_ARGS + 1];
            /* split_argv modifies line with NULL-terminators */
            size_t argc = esp_console_split_argv(src->line, argv, CONFIG_JOLT_CONSOLE_MAX_ARGS);
#if LOG_LOCAL_LEVEL >= 4 /* debug */
            ESP_LOGD(TAG, "%d arguments parsed.", argc);
            for(uint8_t i=0; i < argc; i++) {
                ESP_LOGD(TAG, "argv[%d]: %s", i, argv[i]);
            }
            ESP_LOGD(TAG, "Not an internal command; looking for app of name %s", argv[0]);
#endif
            // TODO: parse passphrase argument
            if( launch_file(argv[0], argc-1, &argv[1], "") ) {
                printf("App failed to launch\n");
                jolt_cli_return(-1);
            }
            else {
                app_call = true;
            }
            break;
        }
        default:
            jolt_cli_return(-1);
            jolt_cli_resume();
            break;
    }

exit:
    return 0; // Don't repeat
}

/**
 * @brief Registers all built-in Jolt commands
 */
static void jolt_cli_cmds_register() {
    esp_console_cmd_t cmd;

    esp_console_register_help_command();

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
        .command = "factory_reset",
        .help = "Perform factory reset",
        .hint = NULL,
        .func = &jolt_cmd_factory_reset,
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
        .command = "lv_obj",
        .help = "Get information about lv_obj's",
        .hint = NULL,
        .func = &jolt_cmd_lv_obj,
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
        .command = "rng",
        .help = "generate cryptographically strong random values",
        .hint = NULL,
        .func = &jolt_cmd_rng,
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

#if JOLT_GUI_TEST_MENU
    cmd = (esp_console_cmd_t) {
        .command = "unlock",
        .help = "Prompts user to enter PIN and unlock device.",
        .hint = NULL,
        .func = &jolt_cmd_unlock,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
#endif

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
