/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "esp_log.h"

#include "hal/radio/wifi.h"
#include "hal/storage/storage.h"
#include "jolt_gui/jolt_gui.h"
#include "hal/lv_drivers/display/ssd1306.h"
#include "jolt_helpers.h"
#include "jolt_globals.h"

static const char TAG[] = "menu_settings";


static lv_action_t menu_wifi_details_create(lv_action_t *btn) {
    char new_ap_info[45];
    get_ap_info(new_ap_info, sizeof(new_ap_info));
    lv_obj_t *t = jolt_gui_scr_text_create("WiFi Details", new_ap_info);
    return LV_RES_OK;
}

static lv_action_t factory_reset_back( lv_obj_t *btn ) {
    jolt_gui_scr_del();
    return LV_RES_INV;
}

static lv_action_t factory_reset_enter( lv_obj_t *btn ) {
    storage_factory_reset( true, NULL );
    return LV_RES_OK;
}

static lv_action_t menu_factory_reset_create(lv_obj_t *btn) {
    lv_obj_t *scr = jolt_gui_scr_menu_create("Factory Reset?");
    jolt_gui_scr_menu_add(scr, NULL, "No", factory_reset_back);
    jolt_gui_scr_menu_add(scr, NULL, "Yes", factory_reset_enter);
    return LV_RES_OK;
}

static const uint8_t brightness_levels[] = {1, 10, 30, 70, 150, 255};

static lv_action_t screen_brightness_save(lv_obj_t *btn) {
    lv_obj_t *slider = jolt_gui_scr_slider_get_slider(lv_obj_get_parent(btn));
    int16_t slider_pos = lv_slider_get_value(slider);
    uint8_t brightness = brightness_levels[slider_pos];
    save_display_brightness(brightness);
    jolt_gui_scr_del();
    return LV_RES_INV;
}

static lv_action_t screen_brightness_update(lv_obj_t *slider) {
    int16_t slider_pos = lv_slider_get_value(slider);
    uint8_t brightness = brightness_levels[slider_pos];
    ssd1306_set_contrast(&disp_hal, brightness);
    return LV_RES_OK;
}

static lv_action_t screen_brightness_back(lv_obj_t *btn) {
    jolt_gui_scr_del();
    ssd1306_set_contrast(&disp_hal, get_display_brightness());
    return LV_RES_INV;
}

static lv_action_t menu_screen_brightness_create() {
    const char title[] = "Brightness";
    uint8_t brightness = get_display_brightness();
    int16_t slider_pos;
    ESP_LOGI(TAG, "Stored brightness: %d", brightness);
    for(slider_pos=0; slider_pos < sizeof(brightness_levels); slider_pos++) {
        if(brightness <= brightness_levels[slider_pos]) {
            break;
        }
    }

    lv_obj_t *scr = jolt_gui_scr_slider_create("Brightness", NULL, screen_brightness_update);
    jolt_gui_scr_slider_set_range(scr, 0, sizeof(brightness_levels)-1);
    jolt_gui_scr_slider_set_value(scr, slider_pos);
    jolt_gui_scr_set_back_action(scr, screen_brightness_back);
    jolt_gui_scr_set_enter_action(scr, screen_brightness_save);
    return LV_RES_OK;
}

lv_action_t menu_settings_create(lv_obj_t *btn) {
    lv_obj_t *scr = jolt_gui_scr_menu_create("Settings");
    jolt_gui_scr_menu_add(scr, NULL, "WiFi", menu_wifi_details_create);
    jolt_gui_scr_menu_add(scr, NULL, "Screen Brightness", menu_screen_brightness_create);
    jolt_gui_scr_menu_add(scr, NULL, "Bluetooth", NULL);
    jolt_gui_scr_menu_add(scr, NULL, "Long Option Name Scrolls", NULL);
    jolt_gui_scr_menu_add(scr, NULL, "Factory Reset", menu_factory_reset_create);
    return 0;
}
