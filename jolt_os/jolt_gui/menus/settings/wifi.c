#if !CONFIG_NO_BLOBS

#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/menus/settings/submenus.h"
#include "hal/radio/wifi.h"
#include "hal/storage/storage.h"
#include "esp_wifi.h"

static const char TAG[]  ="menu/settings/wifi";
static lv_obj_t *scr = NULL;
static lv_obj_t *sw_en = NULL;

static void create_list() {
    jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_WIFI_DETAILS), menu_wifi_details_create);
}

static void destroy_list() {
     jolt_gui_scr_menu_remove(scr, 1, 0);
}

static lv_res_t sw_en_cb(lv_obj_t *btn) {
    uint8_t state;
    state = lv_sw_toggle_anim(sw_en);
    storage_set_u8(state, "user", "wifi_en");

    if( state ) {
        create_list();
        wifi_connect();
    }
    else {
        destroy_list();
        esp_wifi_disconnect();
        esp_wifi_stop();
        esp_wifi_set_mode(WIFI_MODE_NULL);
        esp_wifi_deinit();
    }
    return LV_RES_OK;
}

lv_res_t menu_wifi_create(lv_obj_t *btn) {
    uint8_t wifi_en;
    storage_get_u8(&wifi_en, "user", "wifi_en", 0 );

    scr = jolt_gui_scr_menu_create(gettext(JOLT_TEXT_SETTINGS));
    lv_obj_t *btn_en = jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_WIFI_ENABLE), sw_en_cb);
    sw_en = jolt_gui_scr_menu_add_sw( btn_en );

    if( wifi_en ) {
        lv_sw_on( sw_en );
        create_list();
    }
    else{
        lv_sw_off( sw_en );
    }

    return LV_RES_OK;
}

#endif
