#include "stdio.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "syscore/filesystem.h"
#include "syscore/ymodem.h"
#include "jolt_helpers.h"
#include "lang/lang.h"

#include "jolt_gui/jolt_gui.h"

static const char TAG[] = "cmd_upload";


static void jolt_cmd_upload_cb( lv_obj_t *bar, lv_event_t event ) {
    if( jolt_gui_event.apply == event ){
        jolt_gui_scr_loadingbar_update(bar, NULL, gettext(JOLT_TEXT_INSTALLING), -1);
    }
    else if( jolt_gui_event.value_changed == event ){
        int8_t *progress = NULL;
        progress = jolt_gui_scr_loadingbar_progress_get( bar );
        if(*progress > 0) {
            jolt_gui_scr_loadingbar_update(bar, NULL, gettext(JOLT_TEXT_TRANSFERRING), -1);
        }
    }
}

int jolt_cmd_upload(int argc, char** argv) {
    int return_code;
    char *orig_fullpath = NULL, *orig_fn = NULL;
    FILE *ffd = NULL;
    int rec_res = -1;
    int32_t max_fsize = jolt_fs_free();

    /* Parse filenames */
    jolt_fs_parse_buf(&orig_fullpath, &orig_fn);
    if( NULL == orig_fullpath ) EXIT(-1);

    /* Create loading screen */
    jolt_gui_obj_t *loading_scr = jolt_gui_scr_loadingbar_create("Install");
    jolt_gui_scr_set_event_cb(loading_scr, jolt_cmd_upload_cb);
    int8_t *progress = jolt_gui_scr_loadingbar_autoupdate(loading_scr);
    jolt_gui_scr_loadingbar_update(loading_scr, NULL, gettext(JOLT_TEXT_CONNECTING), 0);

    /* Pre-run cleanup */
    if( jolt_fs_exists(JOLT_FS_TMP_FN) ) remove(JOLT_FS_TMP_FN);

    /* Open tmp file */
    if( NULL == (ffd = fopen(JOLT_FS_TMP_FN, "wb")) ) {
        EXIT_PRINT(-2, "Error opening tmp for receive.");
    }
    
    /* Perform Transfer */
    rec_res = ymodem_receive(ffd, max_fsize, orig_fn, progress);

    SAFE_CLOSE(ffd);

    if(rec_res <= 0){
        /* Failure */
        char buf[64];
        ESP_LOGE(TAG, "Transfer complete, Error=%d", rec_res);
        snprintf(buf, sizeof(buf), "%s=%d", gettext(JOLT_TEXT_ERROR), rec_res);
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
    if( 0 != rename(JOLT_FS_TMP_FN, orig_fullpath) ) {
        EXIT_PRINT(-4, "Unable to rename file. Transfer failed.");
    }

    jolt_h_fn_home_refresh( orig_fn );
    jolt_h_apply_patch( orig_fn );

    EXIT(0);

exit:
    SAFE_CLOSE(ffd);
    if( jolt_fs_exists(JOLT_FS_TMP_FN) ) remove(JOLT_FS_TMP_FN);
    SAFE_FREE(orig_fullpath);

    return return_code;
}


