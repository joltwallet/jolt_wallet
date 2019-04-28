#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"

static const char TAG[] = "menus/settings/language";

static void language_enter( jolt_gui_obj_t *btn, jolt_gui_event_t event ) {
    if( jolt_gui_event.short_clicked == event ) {
        uint8_t index = lv_list_get_btn_index(NULL, btn);
        ESP_LOGI(TAG, "Setting language id %d", jolt_lang_order[index]);
        jolt_lang_set( jolt_lang_order[index] );
        jolt_gui_scr_del();
    }
}

void menu_language_create(jolt_gui_obj_t *btn, jolt_gui_event_t event ) {
    if( jolt_gui_event.short_clicked == event ) {
        jolt_gui_obj_t *scr = jolt_gui_scr_menu_create(gettext(JOLT_TEXT_LANGUAGE));
        for ( jolt_lang_t i=0; i<sizeof(jolt_lang_order); i++ ) {
            if( jolt_lang_available(jolt_lang_order[i]) ){
                jolt_gui_scr_menu_add(scr, NULL, getlangname(i), language_enter);
            }
        }
    }
}

