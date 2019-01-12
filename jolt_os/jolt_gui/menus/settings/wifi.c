#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"

#include "hal/radio/wifi.h"

const char TAG[]  ="menu/settings/wifi";

lv_res_t menu_wifi_details_create(lv_obj_t *btn) {
    char new_ap_info[45];
    get_ap_info(new_ap_info, sizeof(new_ap_info));
    lv_obj_t *t = jolt_gui_scr_text_create(gettext(JOLT_TEXT_WIFI), new_ap_info);
    if(NULL == t) {
        ESP_LOGE(TAG, "Unable to create wifi details text screen");
    }
    return LV_RES_OK;
}

