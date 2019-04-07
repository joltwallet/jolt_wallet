#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"
#include "hal/storage/storage.h"
#include "hal/display.h"

static const char TAG[] = "menus/settings/screen_brightness";


static void screen_brightness_cb(lv_obj_t *slider, lv_event_t event) {
    if( LV_EVENT_VALUE_CHANGED == event ) {
        int16_t slider_pos = lv_slider_get_value(slider);
        set_display_brightness(slider_pos);
    }
    else if( LV_EVENT_SHORT_CLICKED == event || LV_EVENT_CANCEL == event ) {
        int16_t slider_pos = lv_slider_get_value(slider);
        save_display_brightness(slider_pos);
        jolt_gui_scr_del();
    }
}

void menu_screen_brightness_create(lv_obj_t *btn, lv_event_t event) {
    if( LV_EVENT_SHORT_CLICKED == event ) {
        uint8_t slider_pos = get_display_brightness();

        lv_obj_t *scr = jolt_gui_scr_slider_create(gettext(JOLT_TEXT_BRIGHTNESS), NULL, screen_brightness_cb);
        jolt_gui_scr_slider_set_range(scr, 0, DISPLAY_BRIGHTNESS_LEVELS - 1);
        jolt_gui_scr_slider_set_value(scr, slider_pos);
    }
}


