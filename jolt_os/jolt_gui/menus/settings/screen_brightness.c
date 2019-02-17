#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"
#include "hal/storage/storage.h"
#include "hal/display.h"

static const char TAG[] = "menus/settings/screen_brightness";

static lv_res_t screen_brightness_save(lv_obj_t *btn) {
    lv_obj_t *slider = jolt_gui_scr_slider_get_slider(lv_obj_get_parent(btn));
    int16_t slider_pos = lv_slider_get_value(slider);
    save_display_brightness(slider_pos);
    jolt_gui_scr_del();
    return LV_RES_INV;
}

static lv_res_t screen_brightness_update(lv_obj_t *slider) {
    int16_t slider_pos = lv_slider_get_value(slider);
    set_display_brightness(slider_pos);
    return LV_RES_OK;
}

lv_res_t menu_screen_brightness_create() {
    uint8_t slider_pos = get_display_brightness();

    lv_obj_t *scr = jolt_gui_scr_slider_create(gettext(JOLT_TEXT_BRIGHTNESS), NULL, screen_brightness_update);
    jolt_gui_scr_slider_set_range(scr, 0, DISPLAY_BRIGHTNESS_LEVELS - 1);
    jolt_gui_scr_slider_set_value(scr, slider_pos);
    jolt_gui_scr_set_back_action(scr, screen_brightness_save);
    jolt_gui_scr_set_enter_action(scr, screen_brightness_save);
    return LV_RES_OK;
}


