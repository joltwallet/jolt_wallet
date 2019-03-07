/**
 * @file bg.h
 * @brief Background task functionality for lengthier tasks
 * @author Brian Pugh
 */

#ifndef JOLT_SYSCORE_BG_H__
#define JOLT_SYSCORE_BG_H__

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "jolt_gui/jolt_gui.h"

#define CONFIG_JOLT_BG_SIGNAL_QUEUE_LEN 3
#define CONFIG_JOLT_BG_TASK_QUEUE_LEN 10

/**
 * @brief opaque type to handle all Job data
 */
typedef struct jolt_bg_job_t jolt_bg_job_t;

/**
 * @brief Function to execute in the BG task
 */
typedef void (*jolt_bg_task_t)( jolt_bg_job_t *job );

/**
 * @brief Signals to send to the Job task
 * 
 * These signals don't directly cause any action, the Job task must periodically
 * check these signals and react appropriately.
 *
 * Most commonly, during lengthy operations, the task should check for an abort
 * signal, and cleanup/exit ASAP.
 */
enum {
    JOLT_BG_NO_SIGNAL = 0, /**< No signal on queue */
    JOLT_BG_ABORT,         /**< task should cleanup and exit immediately */
};
typedef uint8_t jolt_bg_signal_t;

/**
 * @brief Queues and executes a task to be performed on the BG FreeRTOS task.
 *
 * Since the GUI callbacks must respond as quickly as possible, longer running 
 * actions are packaged up as jobs and send to the BG task to be executed.
 * Within JoltOS, some examples of this are:
 *     * Mnemonic master seed derivation
 *     * HTTP Actions
 *     * Processing CLI commands
 *
 * If a screen is provided, the following will be performed:
 *     1. The screen's back action will be overrided to send a JOLT_BG_ABORT signal to the task.
 * Typically some form of loading screen is used.
 * If your task needs to be passed a screen, but you don't want these actions,
 * pass the screen as part of param.
 *
 * Upon task completion, the Job will clean up itself.
 *
 * @param[in] task function to execute.
 * @param[in] param free parameter to pass to the task
 * @param[in,out] scr Screen for cancel actions.
 *
 * @return ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t jolt_bg_create( jolt_bg_task_t task, void *param, lv_obj_t *scr);


/******************************************************************************
 * The following functions are intended to be used from within the user's
 * jolt_bg_task_t.
 *****************************************************************************/

/**
 * @brief Attempt to read a signal from the job queue. Returns immediately.
 * @param[in] the current job object.
 * @return Signal (if any) popped from the queue.
 */
jolt_bg_signal_t jolt_bg_get_signal( jolt_bg_job_t *job );

/**
 * @brief get the free param pointer
 * @param[in] job Job to get free param from
 * @return the free param pointer
 */
void *jolt_bg_get_param(jolt_bg_job_t *job);

/**
 * @brief get the screen pointer
 * @param[in] job Job to get screen from
 * @return the screen pointer
 */
lv_obj_t *jolt_bg_get_scr(jolt_bg_job_t *job);

/**
 * @brief Convenience function to delete a job's screen. Will set screen pointer to NULL.
 *
 * Generally called at the end of a job function if done using the screen.
 *
 * @param[in, out] job Job
 */
void jolt_bg_del_scr( jolt_bg_job_t *job );

/**
 * @brief If an abort signal has been received, goto "exit" for cleanup
 */
#define JOLT_BG_CHECK_ABORT(job) \
    if( JOLT_BG_ABORT == jolt_bg_get_signal(job) ) { \
        goto exit; \
    }

#endif
