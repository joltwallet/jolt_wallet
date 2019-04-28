#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"

#include "hal/radio/wifi.h"

static const char TAG[]  ="menu/settings/wifi";

void menu_wifi_details_create(jolt_gui_obj_t *btn, jolt_gui_event_t event) {
    if( LV_EVENT_SHORT_CLICKED == event ) {
        char new_ap_info[45];
        get_ap_info(new_ap_info, sizeof(new_ap_info));
        lv_obj_t *t = jolt_gui_scr_text_create(gettext(JOLT_TEXT_WIFI), new_ap_info);
        if(NULL == t) {
            ESP_LOGE(TAG, "Unable to create wifi details text screen");
        }
    }
}

