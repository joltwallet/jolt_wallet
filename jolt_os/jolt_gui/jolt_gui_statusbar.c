#include "esp_log.h"
#include "jolt_gui.h"
#include "jolt_gui_symbols.h"
#include "jolt_gui_statusbar.h"
#include "hal/hw_monitor.h"

static const char TAG[] = "statusbar";
static lv_style_t header_style;
static jolt_gui_obj_t *statusbar_cont;
static jolt_gui_obj_t *statusbar_label;

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
    lv_obj_align(statusbar_label, statusbar_cont,
            LV_ALIGN_IN_RIGHT_MID, -1, 0);
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
    // TODO: custom theme with font spacing 1
    statusbar_label = lv_label_create(statusbar_cont, NULL);
    lv_obj_set_size(statusbar_label, 60, header_style.text.font->line_height);
    lv_label_set_long_mode(statusbar_label, LV_LABEL_LONG_EXPAND);
    lv_label_set_align(statusbar_label, LV_LABEL_ALIGN_RIGHT);

    /* Periodically update the statusbar symbols */
    ESP_LOGD(TAG, "Creating Statusbar Update lv_task");
    lv_task_t *task = lv_task_create(&statusbar_update, CONFIG_JOLT_GUI_STATUSBAR_UPDATE_PERIOD_MS,
            LV_TASK_PRIO_LOW, NULL);
    lv_task_ready( task );
}

jolt_gui_obj_t *statusbar_get_label() {
    return statusbar_label;
}
