#if !CONFIG_NO_BLOBS

#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/menus/settings/submenus.h"
#include "hal/radio/wifi.h"
#include "hal/storage/storage.h"
#include "esp_wifi.h"

static const char TAG[]  ="menu/settings/wifi";
static jolt_gui_obj_t *scr = NULL;
static jolt_gui_obj_t *sw_en = NULL;

static void create_list() {
    jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_WIFI_DETAILS), menu_wifi_details_create);
}

static void destroy_list() {
     jolt_gui_scr_menu_remove(scr, 1, 0);
}

static void sw_en_cb(jolt_gui_obj_t *btn, jolt_gui_event_t event) {
    if( jolt_gui_event.short_clicked == event ) {
        uint8_t state;
        state = lv_sw_toggle(sw_en, true);
        storage_set_u8(state, "user", "wifi_en");

        if( state ) {
            create_list();
            jolt_wifi_start();
        }
        else {
            destroy_list();
            jolt_wifi_stop();
        }
    }
}

void menu_wifi_create(jolt_gui_obj_t *btn, jolt_gui_event_t event) {
    if( jolt_gui_event.short_clicked == event ) {
        uint8_t wifi_en;
        storage_get_u8(&wifi_en, "user", "wifi_en", 0 );

        scr = jolt_gui_scr_menu_create(gettext(JOLT_TEXT_SETTINGS));
        jolt_gui_obj_t *btn_en = jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_WIFI_ENABLE), sw_en_cb);
        sw_en = jolt_gui_scr_menu_add_sw( btn_en );

        if( wifi_en ) {
            ESP_LOGD(TAG, "WiFi is already on");
            lv_sw_on( sw_en, false );
            create_list();
        }
        else{
            ESP_LOGD(TAG, "WiFi is already off");
            lv_sw_off( sw_en, false );
        }
    }
}

#endif
