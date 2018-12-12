#include "jolt_gui_stretch.h"
#include "jolt_gui_loading.h"
#include "hal/storage/storage.h"


void jolt_gui_progress_task_create(jolt_derivation_t *d) {
    d->lv_task = lv_task_create(jolt_progress_update_lv_task,
                100, LV_TASK_PRIO_HIGH, d);
}

/* Task that periodically looks at the progress part of jolt_derivation_t 
 * and updates the progress screen */
void jolt_progress_update_lv_task(jolt_derivation_t *d) {
    if(d->progress <= 100) {
        // The +10 makes it look better
        jolt_gui_scr_loading_update(d->scr, NULL, NULL,
                d->progress + 10);
    }
    else if( NULL != d->cb ) {
        /* Delete loading screen */
        lv_obj_del(d->scr);
        d->scr = NULL;
        /* Delete self lv_task */
        lv_task_del(d->lv_task);
        d->lv_task = NULL;
        /* Call complete callback */
        d->cb(NULL);
    }
}

void jolt_gui_stretch(const char *title, const char *label, uint8_t *key,
        lv_action_t complete_cb) {
    static jolt_derivation_t status;
    status.progress = 0;
    status.data = key;
    status.cb = complete_cb;
    status.scr = jolt_gui_scr_loading_create(title);
    jolt_gui_scr_loading_update(status.scr, NULL, label, 0);

    jolt_gui_progress_task_create(&status);

    // The stretch task is defined with storage because we (optionally) use
    // the ataes132a for hardware-bound encryption.
    xTaskCreate(storage_stretch_task,
            "PinStretch", CONFIG_JOLT_TASK_STACK_SIZE_DERIVATION,
            (void *)&status,
            CONFIG_JOLT_TASK_PRIORITY_DERIVATION, &(status.derivation_task));
}
