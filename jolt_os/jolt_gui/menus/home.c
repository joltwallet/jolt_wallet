#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/lv_theme_jolt.h"
#include "settings.h"
#include "syscore/filesystem.h"
#include "syscore/launcher.h"
#include "jolt_helpers.h"

#include "esp_log.h"

/* Stuff that should be moved somewhere else */
#include "jolt_gui/jolt_gui_qr.h"
#include "jolt_globals.h"
#include <driver/adc.h>

lv_theme_t *jolt_gui_theme = NULL;
/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_action_t jolt_gui_test_qrcode_create(lv_obj_t *btn);
static lv_action_t jolt_gui_test_loading_create(lv_obj_t *btn);
static lv_action_t jolt_gui_test_number_create(lv_obj_t *btn);
static lv_action_t jolt_gui_test_battery_create(lv_obj_t *btn);
static lv_res_t jolt_gui_test_alphabet_create(lv_obj_t * list_btn);

/**********************
 *  STATIC VARIABLES
 **********************/
static const char TAG[] = "menu_home";

static void launch_app_task(void *fn){
    char *fn_c = fn;
    launch_file(fn, 0, NULL); // puts the app back into the gui task
    vTaskDelete(NULL);
}

/* App launching is spawned in a different task because it's a bit intense.
 * Also launch_file is a blocking function*/
static lv_action_t launch_file_proxy(lv_obj_t *btn) {
    char *fn = lv_list_get_btn_text( btn );
    ESP_LOGI(TAG, "Launching %s", fn);
    // todo: make these values in Kconfig
    xTaskCreate(launch_app_task,
            "app_launcher", 12000,
            (void *)fn, 2, NULL);

    return 0;
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
#if PC_SIMULATOR
    jolt_gui_store.main_menu = jolt_gui_menu_create("Main", NULL, "PIN Entry",
            jolt_gui_test_pin_create);
    lv_obj_t *mmlist = jolt_gui_store.main_menu;
    lv_list_add(mmlist, NULL, "Loading Test", jolt_gui_test_loading_create);
    lv_list_add(mmlist, NULL, "Alphabet", jolt_gui_test_alphabet_create);
    lv_list_add(mmlist, NULL, "Numeric Begin", jolt_gui_test_numeric_begin_dp_create);
    lv_list_add(mmlist, NULL, "Numeric End", jolt_gui_test_numeric_end_dp_create);
    lv_list_add(mmlist, NULL, "Numeric Mid", jolt_gui_test_numeric_mid_dp_create);
    lv_list_add(mmlist, NULL, "Text Test", jolt_gui_test_text_create);
    lv_list_add(mmlist, NULL, "Submenu", jolt_gui_test_submenu_create);
#elif ESP_PLATFORM
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
#endif
}

static lv_action_t jolt_gui_test_number_create(lv_obj_t *btn) {
    jolt_gui_scr_num_create( "Number Test", 4, 2, jolt_gui_scr_del); 
    return 0;
}

static lv_action_t jolt_gui_test_qrcode_create(lv_obj_t *btn) {
    const char data[] = "Meow";
    jolt_gui_scr_qr_create("QR Test", "Meow", sizeof(data));
	return 0;
}

static void test_loading_task(void *param) {
    lv_obj_t *scr = (lv_obj_t *)param;
    for(uint8_t i=0;i < 101; vTaskDelay(pdMS_TO_TICKS(1000)), i+=10){
        jolt_gui_sem_take();
        if(i==50){
            jolt_gui_scr_loading_update(scr, "Almost Done", "woof", i);
        }
        else if(i>50){
            jolt_gui_scr_loading_update(scr, NULL, "bark", i);
        }
        else{
            jolt_gui_scr_loading_update(scr, NULL, "meow", i);
        }
        jolt_gui_sem_give();
    }
    lv_obj_del(scr);
    vTaskDelete(NULL);
}

static lv_action_t jolt_gui_test_loading_create(lv_obj_t *btn) {
    lv_obj_t *scr = jolt_gui_scr_loading_create("Loading Test");
    if(NULL == scr){
        ESP_LOGE(TAG, "NULL Loading Screen");
        return 1;
    }
    xTaskCreate(test_loading_task,
                "TestLoading", 4096,
                (void *) scr, 10, NULL);
    return 0;
}


/* Screen that gives info on the battery */
static lv_task_t *test_battery_task_h = NULL;
static lv_obj_t *test_battery_scr = NULL;

static lv_action_t jolt_gui_test_battery_del(lv_obj_t *btn) {
    lv_task_del(test_battery_task_h);
    lv_obj_del(test_battery_scr);
    return 0;
}
void jolt_gui_test_battery_task(void *param) {
    if(NULL != test_battery_scr) {
        lv_obj_del(test_battery_scr);
    }
    int val = adc1_get_raw(JOLT_ADC1_VBATT);
    char buf[40];
    snprintf(buf, sizeof(buf), "Raw Value: %d\nPercentage: %d", val,
            jolt_gui_store.statusbar.indicators[JOLT_GUI_STATUSBAR_INDEX_BATTERY].val);
    test_battery_scr = jolt_gui_scr_text_create("Battery", buf);
    jolt_gui_scr_set_back_action(test_battery_scr, jolt_gui_test_battery_del);
}

static lv_action_t jolt_gui_test_battery_create(lv_obj_t *btn) {
    test_battery_task_h = lv_task_create(jolt_gui_test_battery_task, 300, LV_TASK_PRIO_LOW, NULL);
    return 0;
}

static lv_res_t jolt_gui_test_alphabet_create(lv_obj_t * list_btn) {
    /* Dummy Text Page for Testing */
    jolt_gui_scr_text_create("Alphabet", 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ "
            "abcdefghijklmnopqrstuvwxyz "
            "1234567890 "
            "!@#$%^&*()-+_="
            "{}[];':\",.<>?/\\"
            );
    return LV_RES_OK;
}

