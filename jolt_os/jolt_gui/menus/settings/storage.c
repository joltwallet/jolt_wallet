#include "jolt_gui/jolt_gui.h"
#include "esp_log.h"
#include "esp_spiffs.h"


static const char TAG[] = "gui_storage";

static lv_res_t back_cb( lv_obj_t *btn ) {
    jolt_gui_scr_del();
    return LV_RES_INV;
}

lv_res_t menu_storage_create(lv_obj_t *btn) {
    uint32_t tot, used;
    uint8_t percentage;
    char subtitle[50];

    esp_spiffs_info(NULL, &tot, &used);
    percentage = (used*100)/tot;

    snprintf(subtitle, sizeof(subtitle), "%dKB Free / %dKB Total", used/1024, tot/1024);

    lv_obj_t *scr;
    scr = jolt_gui_scr_loading_create( gettext(JOLT_TEXT_STORAGE) );
    if( NULL == scr ){
        goto exit;
    }
    jolt_gui_scr_set_back_action(scr, back_cb);
    jolt_gui_scr_loading_update(scr, NULL, subtitle, percentage);

exit:
    return LV_RES_OK;
}
