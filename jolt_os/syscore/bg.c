#include "esp_log.h"
#include "bg.h"
#include <esp_timer.h>
#include "freertos/timers.h"

static const char TAG[] = "bg";

static TaskHandle_t task_handle = NULL;
static QueueHandle_t job_queue = NULL;

typedef struct jolt_bg_job_t {
    jolt_bg_task_t task; /**< function to be called */
    void *param;         /**< user parameters to be passed into function */
    QueueHandle_t input; /**< queue to receive jolt_bg_signal_t signals */
    lv_obj_t *scr;       /**< optional screen (usually a loading/preloading screen) */
} jolt_bg_job_t;

/**
 * @brief Get current time in microseconds since boot/deep_sleep_awake
 * @return Time in microseconds
 */
static inline uint64_t get_time_us() {
    return esp_timer_get_time();
}

/**
 * @brief Get current time in milliseconds since boot/deep_sleep_awake
 * @return Time in microseconds
 */
static inline uint64_t get_time_ms() {
    return get_time_us() / 1000;
}

static void add_job_to_queue(jolt_bg_job_t *job) {
    if(!xQueueSend(job_queue, &job, 0)) {
        ESP_LOGE(TAG, "Failed to re-add bg job to the queue");
    }
    free(job);
}

static void bg_task( void *param ) {
    for(;;) {
        int t;
        jolt_bg_job_t job;
        xQueueReceive(job_queue, &job, portMAX_DELAY);

        /* Call the task */
        ESP_LOGD(TAG, "Calling job func");
        t = (job.task)( &job );

        if( t > 0) {
            /* Create a timer that will re-add the job to the queue */
            jolt_bg_job_t *j;
            j = malloc(sizeof(jolt_bg_job_t));
            if( NULL == j ) {
                ESP_LOGE(TAG, "Failed to allocate space for job.");
                goto job_end;
            }
            memcpy(j, &job, sizeof(jolt_bg_job_t));
            xTimerCreate("bgJob", pdMS_TO_TICKS(t), pdFALSE,
                    j, (TimerCallbackFunction_t) add_job_to_queue);
            continue;
        }
job_end:
        /* Delete the Job's input Queue */
        vQueueDelete( job.input );
    }
    vTaskDelete( NULL ); /* Should never reach here */
}

esp_err_t jolt_bg_init( ) {
    /* Create the Job Queue if it doesn't exist */
    if( NULL == job_queue ) {
        job_queue = xQueueCreate( CONFIG_JOLT_BG_TASK_QUEUE_LEN, sizeof(jolt_bg_job_t) );
        if( NULL == job_queue) {
            ESP_LOGE(TAG, "Failed to create bg job queue");
            goto exit;
        }
    }

    /* Create the Job Task if it doesn't exist */
    if( NULL == task_handle) {
        BaseType_t res = xTaskCreate(bg_task, "background",
                CONFIG_JOLT_TASK_STACK_SIZE_BACKGROUND, NULL,
                CONFIG_JOLT_TASK_PRIORITY_BACKGROUND, &task_handle);
        if( errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY == res ) {
            ESP_LOGE(TAG, "Failed to create bg task");
            goto exit;
        }
    }

    return ESP_OK;

exit:
    return ESP_FAIL;
}

static lv_res_t abort_cb( lv_obj_t *btn ) {
    QueueHandle_t queue = jolt_gui_get_param( btn );
    jolt_bg_signal_t signal = JOLT_BG_ABORT;
    switch( xQueueSend(queue, &signal, 0) ) {
        case pdPASS:
            break;
        default:
            ESP_LOGE(TAG, "Failed to send JOLT_BG_ABORT signal");
            break;
    }

    return LV_RES_OK;
}

esp_err_t jolt_bg_create( jolt_bg_task_t task, void *param, lv_obj_t *scr ) {
    esp_err_t err;
    QueueHandle_t input_queue = NULL;

    /* Creates all FreeRTOS objects if they don't already exist */
    err = jolt_bg_init();
    if( ESP_OK != err ) {
        goto exit;
    }

    /* Setup Task Message Queue */
    input_queue = xQueueCreate( CONFIG_JOLT_BG_SIGNAL_QUEUE_LEN, sizeof(jolt_bg_signal_t) );
    if( NULL == job_queue) {
        ESP_LOGE(TAG, "Failed to create bg job queue");
        goto exit;
    }

    /* Configure Job */
    jolt_bg_job_t job;
    job.task  = task;
    job.param = param;
    job.input = input_queue;
    job.scr   = scr;

    /* Register the abort callback to the provided screen */
    if( NULL != scr ) {
        lv_obj_t *back = jolt_gui_scr_set_back_action(scr, abort_cb);
        if( NULL == back ) {
            // failed to set back action
            ESP_LOGE(TAG, "Failed to set back action");
            goto exit;
        }
        jolt_gui_scr_set_back_param(scr, input_queue);
        ESP_LOGI(TAG, "Registered abort back action");
    }

    /* Send the job to the job_queue. Do NOT block to put it on the queue */
    if(!xQueueSend(job_queue, &job, 0)) {
        ESP_LOGE(TAG, "Failed to add bg job to the queue");
        goto exit;
    }

    return ESP_OK;

exit:
    if( input_queue != NULL ){
        vQueueDelete( input_queue );
    }
    return ESP_FAIL;
}

jolt_bg_signal_t jolt_bg_get_signal( jolt_bg_job_t *job ) {
    jolt_bg_signal_t res = JOLT_BG_NO_SIGNAL;
    xQueueReceive(job->input, &res, 0);
    return res;
}

void *jolt_bg_get_param(jolt_bg_job_t *job) {
    return job->param;
}

lv_obj_t *jolt_bg_get_scr(jolt_bg_job_t *job) {
    return job->scr;
}

void jolt_bg_del_scr( jolt_bg_job_t *job ) {
    if( NULL != job->scr) {
        jolt_gui_obj_del( job->scr );
        job->scr = NULL;
    }
}
