#ifndef JOLT_SYSCORE_BG_H__
#define JOLT_SYSCORE_BG_H__

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "jolt_gui/jolt_gui.h"

#define CONFIG_JOLT_BG_SIGNAL_QUEUE_LEN 3
#define CONFIG_JOLT_BG_TASK_QUEUE_LEN 5

typedef struct jolt_bg_job_t jolt_bg_job_t;
typedef void (*jolt_bg_task_t)( jolt_bg_job_t *job );

enum {
    JOLT_BG_NO_SIGNAL = 0,
    JOLT_BG_ABORT,
};
typedef uint8_t jolt_bg_signal_t;

/* Creates and enqueues a job to perform the function in the background task. 
 * If a screen is provided, the following will be performed:
 *     1. A back action will be set on the screen to send a JOLT_BG_ABORT signal to the task
 *     2. Upon task exit, the screen will be deleted. 
 *
 * Returns ESP_OK on success */
esp_err_t jolt_bg_create( jolt_bg_task_t task, void *param, lv_obj_t *scr);

/******************************************************************************
 * The following functions are intended to be used from within the user's
 * jolt_bg_task_t.
 *****************************************************************************/


/* Attempts to get a signal from the job's input queue. Will return immediately */
jolt_bg_signal_t jolt_bg_get_signal( jolt_bg_job_t *job );
void *jolt_bg_get_param(jolt_bg_job_t *job);
lv_obj_t *jolt_bg_get_scr(jolt_bg_job_t *job);

#endif
