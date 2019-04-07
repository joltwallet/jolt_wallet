#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"
#include "hal/storage/storage.h"
#include "hal/led.h"
#include "sdkconfig.h"

// todo: combine the callbacks under different events

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

static void led_brightness_cb(lv_obj_t *slider, lv_event_t event) {
    if( LV_EVENT_VALUE_CHANGED == event ) {
        int16_t slider_pos = lv_slider_get_value(slider);
        uint8_t brightness = brightness_levels[slider_pos];
        jolt_led_set(brightness);
    }
    else if( LV_EVENT_SHORT_CLICKED == event ) {
        int16_t slider_pos = lv_slider_get_value(slider);
        uint8_t brightness = brightness_levels[slider_pos];
        storage_set_u8(brightness, "user", "led_val");
        jolt_gui_scr_del();
    }
}

void menu_led_brightness_create(lv_obj_t *btn, lv_event_t event) {
    
    if( LV_EVENT_SHORT_CLICKED == event ) {
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

        lv_obj_t *scr = jolt_gui_scr_slider_create(gettext(JOLT_TEXT_BRIGHTNESS), NULL, led_brightness_cb);
        jolt_gui_scr_slider_set_range(scr, 0, sizeof(brightness_levels)-1);
        jolt_gui_scr_slider_set_value(scr, slider_pos);
    }
}

