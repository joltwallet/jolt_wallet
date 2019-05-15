#include "jolt_gui/jolt_gui.h"
#include "hal/storage/storage.h"
#include "syscore/ota.h"
#include "syscore/console_helpers.h"
#include "syscore/cli.h"

static const char TAG[] = "cmd_upload_firmware";
static const char progress_label_0[] = "Connecting...";
static const char progress_label_1[] = "Transfering...";
static const char progress_label_2[] = "Installing...";

static void jolt_cmd_upload_firmware_cb( lv_obj_t *bar, lv_event_t event ) {
    if( jolt_gui_event.apply == event ){
        jolt_gui_scr_loadingbar_update(bar, NULL, progress_label_2, -1);
        jolt_gui_scr_del();
    }
    else if( jolt_gui_event.value_changed == event ){
        int8_t *progress = NULL;
        progress = jolt_gui_scr_loadingbar_progress_get( bar );
        if(*progress > 0) {
            jolt_gui_scr_loadingbar_update(bar, NULL, progress_label_1, -1);
        }
    }
}

int jolt_cmd_upload_firmware(int argc, char** argv) {
    esp_err_t err;
    int8_t *progress = NULL;
    jolt_gui_obj_t *loading_scr = NULL;

    jolt_cli_suspend();

    /* Create loading screen */
    loading_scr = jolt_gui_scr_loadingbar_create("JoltOS Update");
    jolt_gui_scr_set_event_cb(loading_scr, jolt_cmd_upload_firmware_cb);
    progress = jolt_gui_scr_loadingbar_autoupdate(loading_scr);
    jolt_gui_scr_loadingbar_update(loading_scr, NULL, progress_label_0, 0);

    /* Begin transfer */
    err = jolt_ota_ymodem( progress );

    if( ESP_OK == err ) {
        ESP_LOGI(TAG, "OTA Success; rebooting...");
        esp_restart();
    }
    else {
        ESP_LOGE(TAG, "OTA Failure");
        jolt_gui_obj_del(loading_scr);
    }

    return 0;
}

