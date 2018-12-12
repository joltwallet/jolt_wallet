#ifndef __JOLT_GUI_STRETCH__
#define __JOLT_GUI_STRETCH__

#include "lvgl/lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define JOLT_DERIVATION_PROGRESS_DONE 101

/* To be used for long computations with a loading screen */
typedef struct jolt_derivation_t{
    lv_obj_t *scr; // Loading Screen Object
    TaskHandle_t derivation_task; // Task performing the stretching
    lv_task_t *lv_task; // Gui update task monitoring progress
    uint8_t progress; // Progress 0 - 100 updated by stretch_task
    uint8_t *data; // 256-bits to stretch and store
    void (*cb) (void *); // Callback to execute upon completion
} jolt_derivation_t;

/* Returns immediately. Will call complete_cb from a lv_task upon completion */
void jolt_gui_stretch(const char *title, const char *label, uint8_t *key,
        lv_action_t complete_cb);

void jolt_progress_update_lv_task(jolt_derivation_t *d);

void jolt_gui_progress_task_create(jolt_derivation_t *d);

#endif
