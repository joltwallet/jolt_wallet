#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"

static const char TAG[] = "jolt_gui_yesno";

jolt_gui_obj_t * jolt_gui_scr_yesno_create(const char *title,
        jolt_gui_event_cb_t yes_cb, jolt_gui_event_cb_t no_cb ) {
    jolt_gui_obj_t *scr = NULL;
    bool success = false;
    JOLT_GUI_CTX{
        scr = BREAK_IF_NULL(jolt_gui_scr_menu_create(title));
        jolt_gui_scr_set_event_cb(scr, no_cb);
        BREAK_IF_NULL(jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_NO), no_cb));
        BREAK_IF_NULL(jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_YES), yes_cb));
        success = true;
    }
    if( !success ){
        ESP_LOGE(TAG, "Failed to create yes/no screen");
        JOLT_GUI_OBJ_DEL_SAFE(scr);
    }
    return scr;
}

