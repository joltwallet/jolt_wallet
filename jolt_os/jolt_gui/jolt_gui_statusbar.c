//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "jolt_gui_statusbar.h"
#include "esp_log.h"
#include "hal/hw_monitor.h"
#include "jolt_gui.h"
#include "jolt_gui_symbols.h"

static const char TAG[] = "statusbar";
static lv_style_t header_style;
static jolt_gui_obj_t *statusbar_cont;
static jolt_gui_obj_t *statusbar_label;

#define JOLT_STATUSBAR_SYMBOL_SPACE 1
#define JOLT_STATUSBAR_BUF_LEN      20

static uint16_t get_statusbar_label_max_width()
{
    char statusbar_symbols[JOLT_STATUSBAR_BUF_LEN] = {0};
    lv_point_t size_res                            = {0};

    strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_LOCK, sizeof( statusbar_symbols ) );
    strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_BLUETOOTH_CONN, sizeof( statusbar_symbols ) );
    strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_WIFI_3, sizeof( statusbar_symbols ) );
#if CONFIG_JOLT_STORE_ATAES132A
    strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_CHIP, sizeof( statusbar_symbols ) );
#endif
    strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_BATTERY_3, sizeof( statusbar_symbols ) );

    lv_txt_get_size( &size_res, statusbar_symbols, &jolt_symbols, JOLT_STATUSBAR_SYMBOL_SPACE, 0, LV_COORD_MAX,
                     LV_TXT_FLAG_NONE );

    ESP_LOGD( TAG, "Statusbar symbols label max width computed: %d", size_res.x );
    return size_res.x;
}

static void statusbar_update( lv_task_t *task )
{
    /* Gets called from a lv_task to update the graphics according to
     * jolt_gui_store.statusbar.indicators */

    char statusbar_symbols[JOLT_STATUSBAR_BUF_LEN] = {0};

    {
        int8_t lock_status;
        lock_status = statusbar_indicators[JOLT_HW_MONITOR_INDEX_LOCK].val;
        if( lock_status != 0 ) strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_LOCK, sizeof( statusbar_symbols ) );
    }

    {
        int8_t bluetooth_level;
        bluetooth_level = statusbar_indicators[JOLT_HW_MONITOR_INDEX_BLUETOOTH].val;
        switch( bluetooth_level ) {
            case JOLT_BLUETOOTH_LEVEL_OFF:
                /* Don't Display Anything */
                break;
            case JOLT_BLUETOOTH_LEVEL_ON:
                strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_BLUETOOTH, sizeof( statusbar_symbols ) );
                break;
            case JOLT_BLUETOOTH_LEVEL_CONN:
                strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_BLUETOOTH_CONN, sizeof( statusbar_symbols ) );
                break;
            default:
                /* Something bad happened */
                break;
        }
    }

    {
        int8_t wifi_level;
        wifi_level = statusbar_indicators[JOLT_HW_MONITOR_INDEX_WIFI].val;
        if( wifi_level == -1 )
            ; /* Display Nothing */
        else if( wifi_level == 0 )
            strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_WIFI_DISCONN, sizeof( statusbar_symbols ) );
        else if( wifi_level <= 55 )
            strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_WIFI_3, sizeof( statusbar_symbols ) );
        else if( wifi_level <= 75 )
            strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_WIFI_2, sizeof( statusbar_symbols ) );
        else
            strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_WIFI_1, sizeof( statusbar_symbols ) );
    }

#if CONFIG_JOLT_STORE_ATAES132A
    strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_CHIP, sizeof( statusbar_symbols ) );
#endif

    {
        int8_t battery_level = statusbar_indicators[JOLT_HW_MONITOR_INDEX_BATTERY].val;
        if( battery_level > 100 )
            strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_BATTERY_CHARGING, sizeof( statusbar_symbols ) );
        else if( battery_level > 70 )
            strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_BATTERY_3, sizeof( statusbar_symbols ) );
        else if( battery_level > 45 )
            strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_BATTERY_2, sizeof( statusbar_symbols ) );
        else if( battery_level > 15 )
            strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_BATTERY_1, sizeof( statusbar_symbols ) );
        else
            strlcat( statusbar_symbols, JOLT_GUI_SYMBOL_BATTERY_EMPTY, sizeof( statusbar_symbols ) );
    }

    // Dont need a semaphore around here because this is called in an lv_task
    lv_label_set_text( statusbar_label, statusbar_symbols );
    lv_obj_align( statusbar_label, statusbar_cont, LV_ALIGN_IN_RIGHT_MID, -1, 0 );

    ESP_LOGD( TAG, "statusbar_label x: %d w: %d", lv_obj_get_x( statusbar_label ),
              lv_obj_get_width( statusbar_label ) );
}

/* Assumes that hardware_monitors have been externally intialized and the
 * hardware_monitor task is running */
void jolt_gui_statusbar_create()
{
    ESP_LOGD( TAG, "Creating Statusbar Object" );

    /* Create StatusBar Container */
    statusbar_cont = RESTART_IF_NULL( lv_cont_create( lv_scr_act(), NULL ) );
    lv_style_copy( &header_style, lv_cont_get_style( statusbar_cont, LV_CONT_STYLE_MAIN ) );
    header_style.body.border.width = 1;
    header_style.body.border.part  = LV_BORDER_BOTTOM;
    header_style.body.border.color = LV_COLOR_BLACK;
    lv_cont_set_style( statusbar_cont, LV_CONT_STYLE_MAIN, &header_style );
    lv_obj_set_size( statusbar_cont, LV_HOR_RES, CONFIG_JOLT_GUI_STATUSBAR_H );

    /* Create Indicator Label*/
    static lv_style_t statusbar_style;
    statusbar_label = RESTART_IF_NULL( lv_label_create( statusbar_cont, NULL ) );
    lv_style_copy( &statusbar_style, lv_obj_get_style( statusbar_label ) );
    statusbar_style.text.letter_space = JOLT_STATUSBAR_SYMBOL_SPACE;
    statusbar_style.text.font         = &jolt_symbols;
    lv_obj_set_style( statusbar_label, &statusbar_style );
    lv_label_set_long_mode( statusbar_label, LV_LABEL_LONG_CROP );
    lv_label_set_align( statusbar_label, LV_LABEL_ALIGN_RIGHT );
    {
        uint16_t label_width;
        label_width = get_statusbar_label_max_width();
        lv_obj_set_size( statusbar_label, label_width, header_style.text.font->line_height );
    }

    /* Periodically update the statusbar symbols */
    ESP_LOGD( TAG, "Creating Statusbar Update lv_task" );
    lv_task_t *task =
            lv_task_create( &statusbar_update, CONFIG_JOLT_GUI_STATUSBAR_UPDATE_PERIOD_MS, LV_TASK_PRIO_LOW, NULL );
    RESTART_IF_NULL( task );
    lv_task_ready( task );
}

jolt_gui_obj_t *jolt_gui_statusbar_get_label() { return statusbar_label; }
