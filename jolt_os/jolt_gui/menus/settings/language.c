#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"

static const char TAG[] = "menus/settings/language";

static lv_res_t language_enter( lv_obj_t *btn ) {
    uint8_t index = lv_list_get_btn_index(NULL, btn);
    ESP_LOGI(TAG, "Setting language id %d", jolt_lang_order[index]);
    jolt_lang_set( jolt_lang_order[index] );
    jolt_gui_scr_del();
    return LV_RES_INV;
}

lv_res_t menu_language_create(lv_obj_t *btn) {
    lv_obj_t *scr = jolt_gui_scr_menu_create(gettext(JOLT_TEXT_LANGUAGE));
    for ( jolt_lang_t i=0; i<sizeof(jolt_lang_order); i++ ) {
        if( jolt_lang_available(jolt_lang_order[i]) ){
            jolt_gui_scr_menu_add(scr, NULL, getlangname(i), language_enter);
        }
    }
    return LV_RES_OK;
}

