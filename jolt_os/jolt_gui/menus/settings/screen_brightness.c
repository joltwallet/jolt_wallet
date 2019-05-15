//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"
#include "hal/storage/storage.h"
#include "hal/display.h"

static const char TAG[] = "menus/settings/screen_brightness";


static void screen_brightness_cb(jolt_gui_obj_t *slider, jolt_gui_event_t event) {
    ESP_LOGD(TAG, "screen_brightness_cb event %d", event);
    if( jolt_gui_event.value_changed == event ) {
        int16_t slider_pos = lv_slider_get_value(slider);
        set_display_brightness(slider_pos);
    }
    else if( jolt_gui_event.short_clicked == event || jolt_gui_event.cancel == event ) {
        int16_t slider_pos = lv_slider_get_value(slider);
        save_display_brightness(slider_pos);
        ESP_LOGD(TAG, "deleting screen");
        jolt_gui_scr_del( slider );
    }
}

void menu_screen_brightness_create(jolt_gui_obj_t *btn, jolt_gui_event_t event) {
    if( jolt_gui_event.short_clicked == event ) {
        uint8_t slider_pos = get_display_brightness();

        jolt_gui_obj_t *scr = jolt_gui_scr_slider_create(gettext(JOLT_TEXT_BRIGHTNESS), NULL, screen_brightness_cb);
        jolt_gui_scr_slider_set_range(scr, 0, DISPLAY_BRIGHTNESS_LEVELS - 1);
        jolt_gui_scr_slider_set_value(scr, slider_pos);
    }
}


