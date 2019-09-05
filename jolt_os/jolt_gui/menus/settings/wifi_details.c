#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_helpers.h"

#include "hal/radio/wifi.h"

static const char TAG[]  ="menu/settings/wifi";

void menu_wifi_details_create(jolt_gui_obj_t *btn, jolt_gui_event_t event) {
    if( LV_EVENT_SHORT_CLICKED == event ) {
        char info[100] = { 0 };
        char *ip_addr = NULL;
        char *ssid = NULL;
        int8_t rssi;

        if( NULL == (ip_addr = jolt_wifi_get_ip())
                || NULL == (ssid = jolt_wifi_get_ssid())
                || 0 == (rssi = jolt_wifi_get_rssi())) {
            snprintf(info, sizeof(info), gettext(JOLT_TEXT_NOT_CONNECTED));
        }
        else {
            snprintf(info, sizeof(info), "SSID: %s\nRSSI: %d\nIP: %s",
                    ssid, rssi, ip_addr );
        }

        lv_obj_t *t = jolt_gui_scr_text_create(gettext(JOLT_TEXT_WIFI), info);
        if(NULL == t) {
            ESP_LOGE(TAG, "Unable to create wifi details text screen");
        }

        SAFE_FREE(ip_addr);
        SAFE_FREE(ssid);
    }
}

