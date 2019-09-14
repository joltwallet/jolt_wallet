/**
 * @bugs
 */

//#define LOG_LOCAL_LEVEL 4

#include "jolt_gui/jolt_gui.h"
#include "esp_log.h"
#include "syscore/filesystem.h"

static const char TAG[] = "gui_storage_details";

static lv_obj_t *btn_file = NULL;
static lv_obj_t *scr_file_options = NULL;
static lv_obj_t *scr_files = NULL;


/**
 * @brief Delete the file indicated by `btn_file`; 
 */
static void delete_file_cb(lv_obj_t *btn, lv_event_t event){
    if( jolt_gui_event.short_clicked == event ) {
        jolt_gui_scr_del(btn);

        const char *fn = lv_list_get_btn_text(btn_file);
        if( NULL == fn ) return;
        ESP_LOGI(TAG, "Deleting file %s", fn);
        char *path = jolt_fs_parse(fn, NULL);
        if( NULL == path ) {
            ESP_LOGE(TAG, "OOM parsing full path");
            return;
        }

        if( jolt_fs_exists(path) ) {
            remove(path);
        }
        else{
            ESP_LOGE(TAG, "Cannot delete %s. Doesn't exist!", fn);
        }
        free(path);
        jolt_gui_scr_del(scr_file_options);
        {
            uint16_t len;
            len = jolt_gui_scr_menu_len(scr_files);
            if(len > 1) {
                jolt_gui_scr_menu_remove(scr_files, btn_file);
            }
            else{
                jolt_gui_scr_del(scr_files);
                scr_files = NULL;
            }
        }
        btn_file = NULL;
    }
}

/**
 * @brief Create yes/no screen to delete a file.
 */
static void delete_file_confirm(lv_obj_t *btn, lv_event_t event) {
    if( jolt_gui_event.short_clicked == event ) {
        jolt_gui_scr_yesno_create(
                gettext(JOLT_TEXT_STORAGE_DETAILS),
                delete_file_cb, NULL);
    }
}

/***
 * @brief List actions user can perform on selected file.
 */
static lv_obj_t *menu_file_options_create() {
    lv_obj_t *scr = NULL;
    lv_obj_t *btn;
    scr = jolt_gui_scr_menu_create(gettext(JOLT_TEXT_STORAGE_DETAILS));
    if( NULL == scr ) goto exit;

    btn = jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_STORAGE_DELETE), delete_file_confirm);
    if( NULL == btn ) goto exit;

exit:
    return scr;
}

/**
 * Create menu to delete file, etc
 */
static void menu_storage_details_options_callback(lv_obj_t *btn, jolt_gui_event_t event) {
    if( jolt_gui_event.short_clicked == event ) {
        btn_file = btn;
        scr_file_options = menu_file_options_create();
    }
}

/**
 * @brief add all the files in the filesystem and their sizes to the menu_scr.
 *
 * @return number of files added. Returns -1 on error
 */
static lv_obj_t *create_and_add_files_to_menu(){
    DIR *dir = NULL;
    struct dirent *ent;
    struct stat sb;
    lv_obj_t *scr = NULL;

    if( !(dir = opendir(JOLT_FS_MOUNTPT)) ) {
        ESP_LOGE(TAG, "Error opening directory\n");
        goto exit;
    }

    /* Read directory entries */
    while ((ent = readdir(dir)) != NULL) {
        char tpath[JOLT_FS_MAX_ABS_PATH_BUF_LEN] = JOLT_FS_MOUNTPT;

        assert( ent->d_type == DT_REG ); /* All objects should be files */

        if( NULL == scr ) {
            scr = jolt_gui_scr_menu_create(gettext(JOLT_TEXT_STORAGE_DETAILS));
            if( NULL == scr ) goto exit;
        }

        lv_obj_t *btn = NULL, *info_label = NULL;
        btn = jolt_gui_scr_menu_add(scr, NULL, ent->d_name, menu_storage_details_options_callback);
        if( NULL == btn ) goto exit;

        /* Parse full filepath */
        if (tpath[strlen(tpath)-1] != '/') strlcat(tpath, "/", sizeof(tpath));
        strlcat(tpath, ent->d_name, sizeof(tpath));
        ESP_LOGD(TAG, "stat path \"%s\"", tpath);

        if (stat(tpath, &sb)){
            info_label = jolt_gui_scr_menu_add_info(btn, "???");
            if( NULL == info_label ) goto exit;
        }
        else {
            uint8_t precision = 0;
            char size_buf[10];
            if( sb.st_size >= MB ) precision = 1;
            ESP_LOGD(TAG, "Precision: %d", precision);
            jolt_bytes_to_hstr(size_buf, sizeof(size_buf), sb.st_size, precision);
            info_label = jolt_gui_scr_menu_add_info(btn, size_buf);
            if( NULL == info_label ) goto exit;
        }
    }

exit:
    if( NULL!=dir ) closedir(dir);
    if( NULL == scr ) {
        scr = jolt_gui_scr_text_create(gettext(JOLT_TEXT_STORAGE_DETAILS),
                gettext(JOLT_TEXT_STORAGE_NO_FILES_FOUND));
    }
    return scr;
}

void menu_storage_details_create(jolt_gui_obj_t *btn, jolt_gui_event_t event) {
    if( jolt_gui_event.short_clicked == event ) {
        scr_files = create_and_add_files_to_menu();
    }
}



