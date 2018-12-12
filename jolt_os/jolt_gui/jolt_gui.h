#ifndef __JOLT_LVGL_GUI_H__
#define __JOLT_LVGL_GUI_H__

#include "lvgl/lvgl.h"
#include "lv_conf.h"
#include "jolt_gui_entry.h"
#include "jolt_gui_first_boot.h"
#include "jolt_gui_loading.h"
#include "jolt_gui_menu.h"
#include "jolt_gui_pin.h"
#include "jolt_gui_qr.h"
#include "jolt_gui_slider.h"
#include "jolt_gui_statusbar.h"
#include "jolt_gui_stretch.h"
#include "jolt_gui_symbols.h"
#include "jolt_gui_debug.h"
#include "menus/home.h"

#if PC_SIMULATOR
    #include "test_stubs.h"
    #include "test_screens.h"
#elif ESP_PLATFORM
    #include "freertos/FreeRTOS.h"
    #include "freertos/queue.h"
    #include "freertos/task.h"
    #include "freertos/semphr.h"
    #include "hal/hw_monitor.h"
    #include "jolttypes.h"
    #include "bipmnemonic.h"

    #include "jelfloader.h"
#endif


#ifndef CONFIG_JOLT_GUI_LOADING_BUF_SIZE
    #define CONFIG_JOLT_GUI_LOADING_BUF_SIZE 30
#endif
/**********************
 *   GLOBAL VARIABLES
 **********************/

/* This whole struct should be moved into globals */
struct {
    bool first_boot;
    SemaphoreHandle_t mutex; // mutex for the entire gui system
    lv_obj_t *main_menu;
    struct {
        lv_group_t *main; // Parent group for user input
        lv_group_t *back; // Group used to handle back button
        lv_group_t *enter;
    } group;
    struct {
        lv_obj_t *container;
        lv_obj_t *label;
        hardware_monitor_t indicators[JOLT_GUI_STATUSBAR_INDEX_NUM];
    } statusbar;
    struct {
        jelfLoaderContext_t *ctx;
        lv_obj_t *scr;
        int argc;
        char **argv;
    } app;
    struct {
        CONFIDENTIAL char passphrase[BM_PASSPHRASE_BUF_LEN]; //currently not active
        CONFIDENTIAL uint256_t pin;
        CONFIDENTIAL char mnemonic[BM_MNEMONIC_BUF_LEN];
        CONFIDENTIAL uint256_t mnemonic_bin;
        CONFIDENTIAL uint512_t master_seed;
    } derivation; // used for message passing; do not keep data here for long.
} jolt_gui_store;

void jolt_gui_sem_take();
void jolt_gui_sem_give();

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/* Run before running jolt_gui_create();
 * Creates all the groups and registers the in-device to the groups */
void jolt_gui_group_create();

/* Creates a title in the top left statusbar. 
 * Allocates and copies the title string. */
lv_obj_t *jolt_gui_obj_title_create(lv_obj_t *parent, const char *title);

/* Creates a parent object for a new screen thats easy to delete */
lv_obj_t *jolt_gui_parent_create();

/* Deletes the container of the currently focused object */
lv_action_t jolt_gui_scr_del();

/* Display Text; Pressing any button returns to previous screen */
lv_obj_t *jolt_gui_scr_text_create(const char *title, const char *body);

/* Calls cb whenever the back button is pressed */
lv_obj_t *jolt_gui_scr_set_back_action(lv_obj_t *parent, lv_action_t cb);
lv_obj_t *jolt_gui_scr_set_enter_action(lv_obj_t *parent, lv_action_t cb);

lv_action_t jolt_gui_send_enter_main(lv_obj_t *btn);
lv_action_t jolt_gui_send_left_main(lv_obj_t *btn);

#endif
