#include "stdio.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "syscore/filesystem.h"
#include "syscore/ymodem.h"
#include "jolt_helpers.h"

#include "jolt_gui/jolt_gui.h"

static const char TAG[] = "cmd_upload";

static const char progress_label_0[] = "Connecting...";
static const char progress_label_1[] = "Transferring...";
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
    int return_code;
    char *tmp_fullpath = NULL;
    char *orig_fullpath = NULL, *orig_fn = NULL;
    FILE *ffd = NULL;
    int rec_res = -1;
    int32_t max_fsize = jolt_fs_free();

    /* Parse filenames */
    if( NULL == (tmp_fullpath = jolt_fs_parse("tmp", NULL)) ) EXIT(-1);
    jolt_fs_parse_buf(&orig_fullpath, &orig_fn);
    if( NULL == orig_fullpath ) EXIT(-1);

    /* Create loading screen */
    jolt_gui_obj_t *loading_scr = jolt_gui_scr_loadingbar_create("Install");
    jolt_gui_scr_set_event_cb(loading_scr, jolt_cmd_upload_cb);
    int8_t *progress = jolt_gui_scr_loadingbar_autoupdate(loading_scr);
    jolt_gui_scr_loadingbar_update(loading_scr, NULL, progress_label_0, 0);

    /* Pre-run cleanup */
    if( jolt_fs_exists(tmp_fullpath) ) remove(tmp_fullpath);

    /* Open tmp file */
    if( NULL == (ffd = fopen(tmp_fullpath, "wb")) ) {
        EXIT_PRINT(-2, "Error opening file \"%s\" for receive.", tmp_fullpath);
    }
    
    /* Perform Transfer */
    rec_res = ymodem_receive(ffd, max_fsize, orig_fn, progress);

    SAFE_CLOSE(ffd);

    if(rec_res <= 0){
        /* Failure */
        char buf[20];
        ESP_LOGE(TAG, "Transfer complete, Error=%d", rec_res);
        snprintf(buf, sizeof(buf), "Error=%d", rec_res);
        jolt_gui_scr_loadingbar_update(loading_scr, NULL, buf, -1);
        /* Allow screen to be deleted via back button */
        jolt_gui_scr_set_event_cb(loading_scr, jolt_gui_event_del);
        EXIT(-5);
    }

    jolt_gui_scr_del(loading_scr);
    printf("\"%s\" Transfer complete, Size=%d Bytes\n", orig_fn, rec_res);

    ESP_LOGI(TAG, "Renaming file");
    if( jolt_fs_exists(orig_fullpath) && 0 != remove(orig_fullpath) ) {
        EXIT_PRINT(-3, "Unable to delete existing file. Transfer failed.");
    }
    if( 0 != rename(tmp_fullpath, orig_fullpath) ) {
        EXIT_PRINT(-4, "Unable to rename file. Transfer failed.");
    }

    jolt_h_fn_home_refresh( orig_fn );

    EXIT(0);

exit:
    SAFE_CLOSE(ffd);
    if( NULL!=tmp_fullpath && jolt_fs_exists(tmp_fullpath) ) remove(tmp_fullpath);
    SAFE_FREE(tmp_fullpath);
    SAFE_FREE(orig_fullpath);

    return return_code;
}


