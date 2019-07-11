#include "stdio.h"
#include "esp_spiffs.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "syscore/filesystem.h"
#include "syscore/ymodem.h"
#include "jolt_helpers.h"

#include "jolt_gui/jolt_gui.h"

static const char TAG[] = "cmd_upload";

static const char progress_label_0[] = "Connecting...";
static const char progress_label_1[] = "Transfering...";
static const char progress_label_2[] = "Installing...";


static void jolt_cmd_upload_cb( lv_obj_t *bar, lv_event_t event ) {
    if( jolt_gui_event.apply == event ){
        jolt_gui_scr_loadingbar_update(bar, NULL, progress_label_2, -1);
    }
    else if( jolt_gui_event.value_changed == event ){
        int8_t *progress = NULL;
        progress = jolt_gui_scr_loadingbar_progress_get( bar );
        if(*progress > 0) {
            jolt_gui_scr_loadingbar_update(bar, NULL, progress_label_1, -1);
        }
    }
}

int jolt_cmd_upload(int argc, char** argv) {

    FILE *ffd = NULL;
    int rec_res = -1;
    int32_t max_fsize = jolt_fs_free();

    /* Open the file */
    char tmp_fn[CONFIG_SPIFFS_OBJ_NAME_LEN] = SPIFFS_BASE_PATH;
    char orig_fn[CONFIG_SPIFFS_OBJ_NAME_LEN] = SPIFFS_BASE_PATH;

    /* Create loading screen */
    jolt_gui_obj_t *loading_scr = jolt_gui_scr_loadingbar_create("Install");
    jolt_gui_scr_set_event_cb(loading_scr, jolt_cmd_upload_cb);
    int8_t *progress = jolt_gui_scr_loadingbar_autoupdate(loading_scr);
    jolt_gui_scr_loadingbar_update(loading_scr, NULL, progress_label_0, 0);

    strcat(tmp_fn, "/tmp");
    if( jolt_fs_exists(tmp_fn) ) {
        remove(tmp_fn);
    }
    ffd = fopen(tmp_fn, "wb");
    if (ffd) {
        strcat(orig_fn, "/");
        rec_res = ymodem_receive(ffd, max_fsize, orig_fn + strlen(orig_fn), progress);
        fclose(ffd);
        //printf("\r\n");
        if (rec_res > 0) {
            jolt_gui_scr_del(loading_scr);
            printf("\"%s\" Transfer complete, Size=%d Bytes\n",
                    orig_fn+strlen(SPIFFS_BASE_PATH), rec_res);
            if( jolt_fs_exists(orig_fn) ) {
                remove(orig_fn);
            }
            ESP_LOGI(TAG, "Renaming file");
            rename(tmp_fn, orig_fn);
            ESP_LOGI(TAG, "File renamed");

            jolt_h_fn_home_refresh( orig_fn );
        }
        else {
            ESP_LOGE(TAG, "Transfer complete, Error=%d", rec_res);
            char buf[20];
            snprintf(buf, sizeof(buf), "Error=%d", rec_res);
            jolt_gui_scr_loadingbar_update(loading_scr, NULL, buf, -1);
            jolt_gui_scr_set_event_cb(loading_scr, jolt_gui_event_del);
            remove(tmp_fn);
        }
    }
    else {
        ESP_LOGE(TAG, "Error opening file \"%s\" for receive.", tmp_fn);
    }
    if( jolt_fs_exists(tmp_fn) ) {
        remove(tmp_fn);
    }

    return 0;
}


