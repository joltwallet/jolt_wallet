#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/test_screens.h"
#include "jolt_gui/menus/settings/settings.h"
#include "syscore/bg.h"
#include "syscore/filesystem.h"
#include "syscore/launcher.h"
#include "jolt_helpers.h"

#include "esp_log.h"

/* Stuff that should be moved somewhere else */
#include "jolt_gui/jolt_gui_qr.h"


/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static const char TAG[] = "menu_home";
static lv_obj_t *main_menu = NULL;

static void launch_app_task(jolt_bg_job_t *job){
    char *fn_c = jolt_bg_get_param(job);
    launch_file(fn_c, 0, NULL, ""); // puts the app back into the gui task
}

/* App launching is spawned in the background task because it's a bit intense.
 * Also launch_file is a blocking function*/
static lv_res_t launch_file_proxy(lv_obj_t *btn) {
    esp_err_t err;
    const char *fn = lv_list_get_btn_text( btn );
    ESP_LOGI(TAG, "Launching %s", fn);

    err = jolt_bg_create( launch_app_task, (void *)fn, NULL);

    return LV_RES_OK;
}

void jolt_gui_menu_home_create() {
    // Find and Register all user apps
    char **fns = NULL;
    uint16_t n_fns = jolt_fs_get_all_elf_fns( &fns );

    main_menu = jolt_gui_scr_menu_create(gettext(JOLT_TEXT_MAIN_MENU_TITLE));
    if( NULL == main_menu ){
        esp_restart();
    }
    lv_obj_t *btn = jolt_gui_scr_set_back_action(main_menu, NULL);
    if( NULL == btn ){
        esp_restart();
    }

    for(uint16_t i=0; i<n_fns; i++) {
        ESP_LOGD(TAG, "Registering App \"%s\" into the GUI", fns[i]);
        jolt_gui_scr_menu_add(main_menu, NULL, fns[i], launch_file_proxy);
    }
    jolt_h_free_char_array(fns, n_fns);

    jolt_gui_scr_menu_add(main_menu, NULL, gettext(JOLT_TEXT_SETTINGS), menu_settings_create);
#if JOLT_GUI_TEST_MENU
    jolt_gui_scr_menu_add(main_menu, NULL, "QR", jolt_gui_test_qrcode_create);
    jolt_gui_scr_menu_add(main_menu, NULL, "Loading", jolt_gui_test_loading_create);
    jolt_gui_scr_menu_add(main_menu, NULL, "Number", jolt_gui_test_number_create);
    jolt_gui_scr_menu_add(main_menu, NULL, "Battery", jolt_gui_test_battery_create);
    jolt_gui_scr_menu_add(main_menu, NULL, "Alphabet", jolt_gui_test_alphabet_create);
    jolt_gui_scr_menu_add(main_menu, NULL, "Https", jolt_gui_test_https_create);
#endif
}

/* Refreshes the home menu.
 * Use cases: call after downloading an app. */
void jolt_gui_menu_home_refresh() {
    if( NULL != main_menu){
        jolt_gui_obj_del(main_menu);
        main_menu = NULL;
    }
    jolt_gui_menu_home_create();
}
