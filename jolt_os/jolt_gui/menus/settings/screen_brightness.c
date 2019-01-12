#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"
#include "hal/storage/storage.h"
#include "hal/lv_drivers/display/ssd1306.h"
#include "hal/display.h"

static const char TAG[] = "menus/settings/screen_brightness";
static const uint8_t brightness_levels[] = {1, 10, 30, 70, 150, 255};

static lv_res_t screen_brightness_save(lv_obj_t *btn) {
    lv_obj_t *slider = jolt_gui_scr_slider_get_slider(lv_obj_get_parent(btn));
    int16_t slider_pos = lv_slider_get_value(slider);
    uint8_t brightness = brightness_levels[slider_pos];
    save_display_brightness(brightness);
    jolt_gui_scr_del();
    return LV_RES_INV;
}

static lv_res_t screen_brightness_update(lv_obj_t *slider) {
    int16_t slider_pos = lv_slider_get_value(slider);
    uint8_t brightness = brightness_levels[slider_pos];
    ssd1306_set_contrast(&disp_hal, brightness);
    return LV_RES_OK;
}

static lv_res_t screen_brightness_back(lv_obj_t *btn) {
    jolt_gui_scr_del();
    ssd1306_set_contrast(&disp_hal, get_display_brightness());
    return LV_RES_INV;
}

lv_res_t menu_screen_brightness_create() {
    uint8_t brightness = get_display_brightness();
    int16_t slider_pos;
    ESP_LOGI(TAG, "Stored brightness: %d", brightness);
    for(slider_pos=0; slider_pos < sizeof(brightness_levels); slider_pos++) {
        if(brightness <= brightness_levels[slider_pos]) {
            break;
        }
    }

    lv_obj_t *scr = jolt_gui_scr_slider_create(gettext(JOLT_TEXT_BRIGHTNESS), NULL, screen_brightness_update);
    jolt_gui_scr_slider_set_range(scr, 0, sizeof(brightness_levels)-1);
    jolt_gui_scr_slider_set_value(scr, slider_pos);
    jolt_gui_scr_set_back_action(scr, screen_brightness_back);
    jolt_gui_scr_set_enter_action(scr, screen_brightness_save);
    return LV_RES_OK;
}


