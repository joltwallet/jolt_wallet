//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "esp_log.h"
#include "jolt_gui.h"
#include "jolt_gui_symbols.h"
#include "jolt_gui_statusbar.h"
#include "hal/hw_monitor.h"

static const char TAG[] = "statusbar";
static lv_style_t header_style;
static jolt_gui_obj_t *statusbar_cont;
static jolt_gui_obj_t *statusbar_label;

#define JOLT_STATUSBAR_SYMBOL_SPACE 1

#if 0
static uint16_t get_symbol_width( char *sym ) {
    const lv_font_t *font = header_style.text.font;
    uint32_t i = 0;
    uint32_t letter;
    letter = lv_txt_encoded_next(sym, &i);
    return lv_font_get_width(font, letter);
    return lv_font_get_glyph_width(font, uint32_t letter, uint32_t letter_next);
}
#endif
static uint16_t get_statusbar_label_max_width(){
    char statusbar_symbols[20] = { 0 };
    char *ptr = statusbar_symbols;
    lv_point_t size_res = { 0 };

    strcpy(ptr, JOLT_GUI_SYMBOL_LOCK);
    ptr += 3;

    strcpy(ptr, JOLT_GUI_SYMBOL_BLUETOOTH_CONN);
    ptr += 3;

    strcpy(ptr, JOLT_GUI_SYMBOL_WIFI_3);
    ptr += 3;

#if CONFIG_JOLT_STORE_ATAES132A
    strcpy(ptr, JOLT_GUI_SYMBOL_CHIP);
    ptr += 3;
#endif

    strcpy(ptr, JOLT_GUI_SYMBOL_BATTERY_3);
    ptr += 3;

    lv_txt_get_size(&size_res, statusbar_symbols, &jolt_symbols,
            JOLT_STATUSBAR_SYMBOL_SPACE, 0, LV_COORD_MAX, LV_TXT_FLAG_NONE);

    return size_res.x;
}

static void statusbar_update( lv_task_t *task ) {
    /* Gets called from a lv_task to update the graphics according to 
     * jolt_gui_store.statusbar.indicators */

    char statusbar_symbols[20] = { 0 };
    char *ptr = statusbar_symbols;

    int8_t lock_status;
    lock_status = statusbar_indicators[JOLT_HW_MONITOR_INDEX_LOCK].val;
    if( lock_status == 0 ) {
    }
    else {
        strcpy(ptr, JOLT_GUI_SYMBOL_LOCK);
        ptr += 3;
    }

    int8_t bluetooth_level;
    bluetooth_level = statusbar_indicators[JOLT_HW_MONITOR_INDEX_BLUETOOTH].val;
    switch( bluetooth_level ) {
        case JOLT_BLUETOOTH_LEVEL_OFF:
            /* Don't Display Anything */
            break;
        case JOLT_BLUETOOTH_LEVEL_ON:
            strcpy(ptr, JOLT_GUI_SYMBOL_BLUETOOTH);
            ptr += 3;
            break;
        case JOLT_BLUETOOTH_LEVEL_CONN:
            strcpy(ptr, JOLT_GUI_SYMBOL_BLUETOOTH_CONN);
            ptr += 3;
            break;
        default:
            /* Something bad happened */
            break;
    }

    int8_t wifi_level;
    wifi_level = statusbar_indicators[JOLT_HW_MONITOR_INDEX_WIFI].val;
    if( wifi_level == -1 ) {
        /* Display Nothing */
    }
    else if( wifi_level == 0 ) {
        strcpy(ptr, JOLT_GUI_SYMBOL_WIFI_DISCONN);
        ptr += 3;
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

#if CONFIG_JOLT_STORE_ATAES132A
    strcpy(ptr, JOLT_GUI_SYMBOL_CHIP);
    ptr += 3;
#endif

    int8_t battery_level = statusbar_indicators[JOLT_HW_MONITOR_INDEX_BATTERY].val; 
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
    lv_label_set_text(statusbar_label, statusbar_symbols);
    lv_obj_align(statusbar_label, statusbar_cont, LV_ALIGN_IN_RIGHT_MID, -1, 0);

    ESP_LOGD(TAG, "statusbar_label x: %d w: %d", lv_obj_get_x(statusbar_label), lv_obj_get_width(statusbar_label));
}

/* Assumes that hardware_monitors have been externally intialized and the 
 * hardware_monitor task is running */
void statusbar_create() {
    ESP_LOGD(TAG, "Creating Statusbar Object");

    /* Create StatusBar Container */
    statusbar_cont = lv_cont_create(lv_scr_act(), NULL);
    lv_style_copy(&header_style, lv_cont_get_style(statusbar_cont, LV_CONT_STYLE_MAIN) );
    header_style.body.border.width = 1;
    header_style.body.border.part = LV_BORDER_BOTTOM;
    header_style.body.border.color = LV_COLOR_BLACK;
    lv_cont_set_style(statusbar_cont, LV_CONT_STYLE_MAIN, &header_style);
    lv_obj_set_size(statusbar_cont, LV_HOR_RES, CONFIG_JOLT_GUI_STATUSBAR_H);

    /* Create Indicator Label*/
    static lv_style_t statusbar_style;
    statusbar_label = lv_label_create(statusbar_cont, NULL);
    lv_style_copy(&statusbar_style, lv_obj_get_style(statusbar_label));
    statusbar_style.text.letter_space = JOLT_STATUSBAR_SYMBOL_SPACE;
    statusbar_style.text.font = &jolt_symbols;
    lv_obj_set_style(statusbar_label, &statusbar_style);
    lv_label_set_long_mode(statusbar_label, LV_LABEL_LONG_CROP);
    lv_label_set_align(statusbar_label, LV_LABEL_ALIGN_RIGHT);
    {
        uint16_t label_width;
        label_width = get_statusbar_label_max_width();
        lv_obj_set_size(statusbar_label, label_width,
                header_style.text.font->line_height);
    }

    /* Periodically update the statusbar symbols */
    ESP_LOGD(TAG, "Creating Statusbar Update lv_task");
    lv_task_t *task = lv_task_create(&statusbar_update,
            CONFIG_JOLT_GUI_STATUSBAR_UPDATE_PERIOD_MS,
            LV_TASK_PRIO_LOW, NULL);
    lv_task_ready( task );
}

jolt_gui_obj_t *statusbar_get_label() {
    return statusbar_label;
}
