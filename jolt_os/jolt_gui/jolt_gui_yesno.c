
#define LOG_LOCAL_LEVEL 4

#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"

static const char TAG[] = "jolt_gui_yesno";

static void default_no_cb(lv_obj_t *btn, lv_event_t event) {
    if( jolt_gui_event.short_clicked == event
            || jolt_gui_event.cancel == event) {
        ESP_LOGD(TAG, "Deleting yes/no screen via %s and event %d", __func__, event);
        jolt_gui_scr_del(btn);
    }
}

jolt_gui_obj_t * jolt_gui_scr_yesno_create(const char *title,
        jolt_gui_event_cb_t yes_cb, jolt_gui_event_cb_t no_cb ) {
    jolt_gui_obj_t *scr = NULL;
    bool success = false;
    JOLT_GUI_CTX{
        scr = BREAK_IF_NULL(jolt_gui_scr_menu_create(title));
        if( NULL == no_cb ) no_cb = default_no_cb;
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

