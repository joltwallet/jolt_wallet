#include "jolt_gui/jolt_gui.h"
#include "hal/storage/storage.h"
#include "syscore/ota.h"
#include "syscore/console_helpers.h"

static int8_t progress;
static lv_task_t *task;
static lv_obj_t *loading_scr;

static const char TAG[] = "cmd_upload_firmware";
static const char progress_label_0[] = "Connecting...";
static const char progress_label_1[] = "Transfering...";
static const char progress_label_2[] = "Installing...";

static void cmd_upload_firmware_progress_update_lv_task(void *param) {
    const char *label;
    if( NULL == loading_scr ){
        loading_scr = jolt_gui_scr_loading_create("JoltOS Update");
    }
    if(progress <= 100 && progress >= 0) {
        if(progress == 0)
            label = progress_label_0;
        else if(progress < 100)
            label = progress_label_1;
        else
            label = progress_label_2;
        jolt_gui_scr_loading_update(loading_scr, NULL, label, progress);
    }
    else{
        /* Delete loading screen */
        lv_obj_del(loading_scr);
        loading_scr = NULL;
        /* Delete self lv_task */
        lv_task_del(task);
        task = NULL;
    }
}

int jolt_cmd_upload_firmware(int argc, char** argv) {
    esp_err_t err;

    progress = 0;
    task = NULL;

    cmd_upload_firmware_progress_update_lv_task( NULL );
    task = lv_task_create(cmd_upload_firmware_progress_update_lv_task,
                100, LV_TASK_PRIO_HIGH, NULL);
    vTaskDelay(pdMS_TO_TICKS(80)); // Give the GL a moment to draw screen

    err = jolt_ota_ymodem( &progress );

    if( ESP_OK == err ) {
        ESP_LOGI(TAG, "OTA Success; rebooting...");
        esp_restart();
    }
    else {
        ESP_LOGE(TAG, "OTA Failure");
        progress = -1;
    }

    return 0;
}

