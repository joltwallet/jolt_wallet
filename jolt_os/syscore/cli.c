//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

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
static FILE * src_lock_in = NULL; /**< Pointer to the stdin stream that took the src_lock */
static SemaphoreHandle_t job_in_progress = NULL; /**< Mutex indicates if a job is underway or not */
static QueueHandle_t msg_queue = NULL;           /**< Queue that holds char ptrs to data sent over CLI. */
static QueueHandle_t ret_val_queue = NULL;       /**< Queue that the cmd populates when complete. */

void jolt_cli_init() {
    msg_queue = xQueueCreate(5, sizeof(jolt_cli_src_t));
    if( NULL == msg_queue) goto exit;

    job_in_progress = xSemaphoreCreateMutex();
    if( NULL == job_in_progress ) goto exit;

    src_lock = xSemaphoreCreateBinary();
    if( NULL == src_lock ) goto exit;

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

    /* if no job is actually being executed, we can give up the semaphore */
    /* if cmd is already being executed, give up the source lock.
     * else don't give up the source lock.
     */
    stdin = src.in;
    stdout = src.out;
    stderr = src.err;

    if( xSemaphoreTake(job_in_progress, 0) ) {
        /* No cmd currently executed */
    }
    else {
        /* There is currently a cmd being executed. For convenience, we are
         * freeing the src_lock here so the user doesn't have to.*/
        jolt_cli_give_src_lock();
    }
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
    if(timeout < 0) delay = portMAX_DELAY;
    else delay = pdMS_TO_TICKS(timeout);

    portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&myMutex);
    if( xQueueReceive(msg_queue, src, 0) ) {
        portEXIT_CRITICAL(&myMutex);
        res = true;
    }
    else {
        if(!jolt_cli_give_src_lock()){
            //ESP_LOGD(TAG, "Failed to give src lock");
        }
        portEXIT_CRITICAL(&myMutex);
        res = (pdTRUE == xQueueReceive(msg_queue, src, delay));
    }

    return res;
}

static bool jolt_cli_take_src_lock(TickType_t ticks_to_wait){
    bool res;
    //ESP_LOGD(TAG, "Taking src_lock;");
    res = xSemaphoreTake(src_lock, ticks_to_wait);
    //if(res) ESP_LOGD(TAG, "Took src_lock");
    //else ESP_LOGD(TAG, "Failed to take src_lock");
    return res;
}

static bool jolt_cli_give_src_lock(){
    bool res;
    //ESP_LOGD(TAG, "Giving src_lock;");
    res = xSemaphoreGive( src_lock );
    //ESP_LOGD(TAG, "Gave src_lock");
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
            jolt_cli_take_src_lock(portMAX_DELAY);
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
    if( JOLT_CLI_NON_BLOCKING == val ) {
        return;
    }

    portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&myMutex);
    if( xSemaphoreTake(job_in_progress, 0) ) {
        /* No cmd currently executed */
        xSemaphoreGive(job_in_progress);
        portEXIT_CRITICAL(&myMutex);
    }
    else {
        /* There is currently a cmd being executed. Queue up the value. */
        portEXIT_CRITICAL(&myMutex);
        xQueueSend(ret_val_queue, &val, portMAX_DELAY);
        if( 0 != val ) {
            printf("Command returned non-zero error code: %d\n", val);
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

        /* Start Job. This should ALWAYS return immediately since this function is
         * single threaded and is the only place taking/giving job_in_progress. */
        xSemaphoreTake(job_in_progress, portMAX_DELAY);

        /* Dispatch job to bg */
        jolt_bg_create(jolt_cli_process_task, &src, NULL);

        /* Block until job finished. */
        xQueueReceive(ret_val_queue, &ret_val, portMAX_DELAY);

        /* Release source mutex */
        jolt_cli_give_src_lock();

        /* Cmd Finished */
        xSemaphoreGive(job_in_progress);
    }
}

/**
 * @brief Executes the command string in the bg task.
 * @return Time in mS to wait before repeating.
 */
static int32_t jolt_cli_process_task(jolt_bg_job_t *bg_job) {
    jolt_cli_src_t *src = jolt_bg_get_param(bg_job); 
    jolt_cli_src_t src_obj;
    int ret = 0;
    esp_err_t err;

    /* Make a local copy of src */
    memcpy(&src_obj, src, sizeof(jolt_cli_src_t));
    src = &src_obj;

    if(NULL == src->line) goto exit;

    if( src->in ) stdin = src->in;
    if( src->out ) stdout = src->out;
    if( src->err ) stderr = src->err;

    /* Try to run the command */
    err = esp_console_run(src->line, &ret);
    switch( err ) {
        case ESP_ERR_NOT_FOUND: {
            /* The command could be an app to run console commands from */
            static char *argv[CONFIG_JOLT_CONSOLE_MAX_ARGS + 1];
            /* split_argv modifies line with NULL-terminators */
            size_t argc = esp_console_split_argv(src->line, argv, CONFIG_JOLT_CONSOLE_MAX_ARGS);
#if ESP_LOG_LEVEL >= ESP_LOG_DEBUG
            ESP_LOGD(TAG, "%d arguments parsed.", argc);
            for(uint8_t i=0; i < argc; i++) {
                ESP_LOGD(TAG, "argv[%d]: %s", i, argv[i]);
            }
            ESP_LOGD(TAG, "Not an internal command; looking for app of name %s", argv[0]);
#endif
            // todo, parse passphrase argument
            if( launch_file(argv[0], argc-1, &argv[1], "") ) {
                printf("Unsuccessful command\n");
                jolt_cli_return(-1);
            }
            else if( 1 == argc ){
                /* Just launch the GUI */
                jolt_cli_return(0);
            }
            break;
        }
        case ESP_ERR_INVALID_ARG:
            /* command was empty */
            jolt_cli_return(-1);
            break;
        case ESP_OK:

            if( JOLT_CLI_NON_BLOCKING == ret ) {
                break;
            }
            jolt_cli_return(ret);
            break;
        default:
            jolt_cli_return(-1);
            break;
    }

exit:
    /* De-allocate memory */
    if( src->line ) {
        ESP_LOGD(TAG, "%d: Freeing src->line %p", __LINE__, src->line);
        free(src->line);
    }
    return 0;
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
