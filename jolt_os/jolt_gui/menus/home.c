#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/lv_theme_jolt.h"
#include "jolt_gui/test_screens.h"
#include "settings.h"
#include "syscore/filesystem.h"
#include "syscore/launcher.h"
#include "jolt_helpers.h"

#include "esp_log.h"

/* Stuff that should be moved somewhere else */
#include "jolt_gui/jolt_gui_qr.h"
#include "jolt_globals.h"

lv_theme_t *jolt_gui_theme = NULL;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static const char TAG[] = "menu_home";

static void launch_app_task(void *fn){
    char *fn_c = fn;
    launch_file(fn_c, 0, NULL); // puts the app back into the gui task
    vTaskDelete(NULL);
}

/* App launching is spawned in a different task because it's a bit intense.
 * Also launch_file is a blocking function*/
static lv_res_t launch_file_proxy(lv_obj_t *btn) {
    const char *fn = lv_list_get_btn_text( btn );
    ESP_LOGI(TAG, "Launching %s", fn);

    xTaskCreate(launch_app_task,
            "app_launcher", CONFIG_JOLT_TASK_STACK_SIZE_APP_LAUNCHER,
            (void *)fn, CONFIG_JOLT_TASK_PRIORITY_APP_LAUNCHER, NULL);

    return LV_RES_OK;
}

void jolt_gui_menu_home_create() {
    /* Set Jolt ssd1306 theme */
    jolt_gui_theme = lv_theme_jolt_init(100, NULL);
    lv_theme_set_current(jolt_gui_theme);  

    // Don't need to set group since indev driver sends direct keypresses
    lv_obj_t *btn_back = lv_btn_create(lv_scr_act(), NULL);
    lv_btn_set_action(btn_back, LV_BTN_ACTION_CLICK, jolt_gui_scr_del);
    lv_group_add_obj(jolt_gui_store.group.back, btn_back);

    /* Create StatusBar */
    statusbar_create();

    /*Create the list*/
    if( true && jolt_gui_store.first_boot ) {
        jolt_gui_first_boot_create();
    }
    else {
        // Find and Register all user apps
        char **fns = NULL;
        uint16_t n_fns = jolt_fs_get_all_elf_fns( &fns );

        jolt_gui_store.main_menu = jolt_gui_scr_menu_create("Main");
        for(uint16_t i=0; i<n_fns; i++) {
            ESP_LOGD(TAG, "Registering App \"%s\" into the GUI", fns[i]);
            jolt_gui_scr_menu_add(jolt_gui_store.main_menu, NULL, fns[i], launch_file_proxy);
        }
        jolt_h_free_char_array(fns, n_fns);

        jolt_gui_scr_menu_add(jolt_gui_store.main_menu, NULL, "Settings", menu_settings_create);
#if JOLT_GUI_TEST_MENU
        jolt_gui_scr_menu_add(jolt_gui_store.main_menu, NULL, "QR", jolt_gui_test_qrcode_create);
        jolt_gui_scr_menu_add(jolt_gui_store.main_menu, NULL, "Loading", jolt_gui_test_loading_create);
        jolt_gui_scr_menu_add(jolt_gui_store.main_menu, NULL, "Number", jolt_gui_test_number_create);
        jolt_gui_scr_menu_add(jolt_gui_store.main_menu, NULL, "Battery", jolt_gui_test_battery_create);
        jolt_gui_scr_menu_add(jolt_gui_store.main_menu, NULL, "Alphabet", jolt_gui_test_alphabet_create);
        jolt_gui_scr_menu_add(jolt_gui_store.main_menu, NULL, "Dummy 4", NULL);
        jolt_gui_scr_menu_add(jolt_gui_store.main_menu, NULL, "Dummy 5", NULL);
        jolt_gui_scr_menu_add(jolt_gui_store.main_menu, NULL, "Dummy 6", NULL);
#endif

    }
}
