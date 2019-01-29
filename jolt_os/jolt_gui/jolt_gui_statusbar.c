#include "jolt_gui.h"
#include "jolt_gui_symbols.h"
#include "jolt_gui_statusbar.h"

static void statusbar_update() {
    /* Gets called from a lv_task to update the graphics according to 
     * jolt_gui_store.statusbar.indicators */

    char statusbar_symbols[20] = { 0 };
    char *ptr = statusbar_symbols;

    uint8_t lock_status;
    lock_status = jolt_gui_store.statusbar.indicators[JOLT_GUI_STATUSBAR_INDEX_LOCK].val;
    if( lock_status == 0 ) {
    }
    else {
        strcpy(ptr, JOLT_GUI_SYMBOL_LOCK);
        ptr += 3;
    }

    uint8_t bluetooth_level;
    bluetooth_level = jolt_gui_store.statusbar.indicators[JOLT_GUI_STATUSBAR_INDEX_BLUETOOTH].val;
    if( bluetooth_level == 0 ){
    }
    else {
        strcpy(ptr, JOLT_GUI_SYMBOL_BLUETOOTH);
        ptr += 3;
    }

    uint8_t wifi_level;
    wifi_level = jolt_gui_store.statusbar.indicators[JOLT_GUI_STATUSBAR_INDEX_WIFI].val;
    if( wifi_level == 0 ) {
    }
    else if (wifi_level <= 55) {
        strcpy(ptr, JOLT_GUI_SYMBOL_WIFI_3);
        ptr += 3;
    }
    else if (wifi_level <= 75) {
        strcpy(ptr, JOLT_GUI_SYMBOL_WIFI_2);
        ptr += 3;
    }
    else {
        strcpy(ptr, JOLT_GUI_SYMBOL_WIFI_1);
        ptr += 3;
    }

    uint8_t battery_level = jolt_gui_store.statusbar.indicators[JOLT_GUI_STATUSBAR_INDEX_BATTERY].val; 
    if( battery_level > 100 ) {
        strcpy(ptr, JOLT_GUI_SYMBOL_BATTERY_CHARGING);
    }
    else if( battery_level > 70 ) {
        strcpy(ptr, JOLT_GUI_SYMBOL_BATTERY_3);
    }
    else if( battery_level > 45 ) {
        strcpy(ptr, JOLT_GUI_SYMBOL_BATTERY_2);
    }
    else if( battery_level > 15 ) {
        strcpy(ptr, JOLT_GUI_SYMBOL_BATTERY_1);
    }
    else {
        strcpy(ptr, JOLT_GUI_SYMBOL_BATTERY_EMPTY);
    }
    ptr += 3;

    // Dont need a semaphore around here because this is called in an lv_task
    lv_label_set_text(jolt_gui_store.statusbar.label, statusbar_symbols);
    lv_obj_align(jolt_gui_store.statusbar.label, jolt_gui_store.statusbar.container,
            LV_ALIGN_IN_RIGHT_MID, -1, 0);
}

/* Assumes that hardware_monitors have been externally intialized and the 
 * hardware_monitor task is running */
void statusbar_create() {
    /* Create StatusBar Container */
    static lv_style_t header_style;
    jolt_gui_store.statusbar.container = lv_cont_create(lv_scr_act(), NULL);
    lv_style_copy(&header_style, 
            lv_cont_get_style(jolt_gui_store.statusbar.container) );
    header_style.body.border.width = 1;
    header_style.body.border.part = LV_BORDER_BOTTOM;
    header_style.body.border.color = LV_COLOR_BLACK;
    lv_cont_set_style(jolt_gui_store.statusbar.container, &header_style);
    lv_obj_set_size(jolt_gui_store.statusbar.container,
            LV_HOR_RES, CONFIG_JOLT_GUI_STATUSBAR_H);

    /* Select Jolt GUI Symbol Font for indicators */
    static lv_style_t indicator_style;
    lv_style_copy(&indicator_style, &header_style);
    indicator_style.text.font = &jolt_gui_symbols;
    indicator_style.body.padding.hor = 1;

    /* Create Indicator Label*/
    jolt_gui_store.statusbar.label = lv_label_create(jolt_gui_store.statusbar.container,
            NULL);
    lv_label_set_style(jolt_gui_store.statusbar.label, &indicator_style);
    lv_obj_set_size(jolt_gui_store.statusbar.label,
            LV_HOR_RES - CONFIG_JOLT_GUI_TITLE_W, 
            indicator_style.text.font->h_px);

    /* Periodically update the statusbar symbols */
    statusbar_update();
    lv_task_create(&statusbar_update, CONFIG_JOLT_GUI_STATUSBAR_UPDATE_PERIOD_MS,
            LV_TASK_PRIO_LOW, NULL);
}

