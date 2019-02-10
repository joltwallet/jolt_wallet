#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"
#include "hal/storage/storage.h"
#include "hal/lv_drivers/display/ssd1306.h"
#include "hal/led.h"
#include "sdkconfig.h"


static const char TAG[] = "menus/settings/screen_brightness";
#if CONFIG_JOLT_TOUCH_LED_INVERT
static const uint8_t brightness_levels[] = {255, 150, 70, 30, 10, 0};
#else
static const uint8_t brightness_levels[] = {0, 10, 30, 70, 150, 255};
#endif

static uint8_t led_brightness_get() {
    uint8_t val;
    storage_get_u8(&val, "user", "led_val", CONFIG_JOLT_TOUCH_LED_DEFAULT_DUTY );
    return val;
}

static lv_res_t led_brightness_save(lv_obj_t *btn) {
    lv_obj_t *slider = jolt_gui_scr_slider_get_slider(lv_obj_get_parent(btn));
    int16_t slider_pos = lv_slider_get_value(slider);
    uint8_t brightness = brightness_levels[slider_pos];
    storage_set_u8(brightness, "user", "led_val");
    jolt_gui_scr_del();
    return LV_RES_INV;
}

static lv_res_t led_brightness_update(lv_obj_t *slider) {
    int16_t slider_pos = lv_slider_get_value(slider);
    uint8_t brightness = brightness_levels[slider_pos];
    jolt_led_set(brightness);
    return LV_RES_OK;
}

lv_res_t menu_led_brightness_create() {
    uint8_t brightness = led_brightness_get();
    int16_t slider_pos;
    ESP_LOGI(TAG, "Stored brightness: %d", brightness);
    for(slider_pos=0; slider_pos < sizeof(brightness_levels); slider_pos++) {
#if CONFIG_JOLT_TOUCH_LED_INVERT
        if(brightness >= brightness_levels[slider_pos]) {
            break;
        }
#else
        if(brightness <= brightness_levels[slider_pos]) {
            break;
        }
#endif
    }

    lv_obj_t *scr = jolt_gui_scr_slider_create(gettext(JOLT_TEXT_BRIGHTNESS), NULL, led_brightness_update);
    jolt_gui_scr_slider_set_range(scr, 0, sizeof(brightness_levels)-1);
    jolt_gui_scr_slider_set_value(scr, slider_pos);
    jolt_gui_scr_set_back_action(scr, led_brightness_save);
    jolt_gui_scr_set_enter_action(scr, led_brightness_save);
    return LV_RES_OK;
}


