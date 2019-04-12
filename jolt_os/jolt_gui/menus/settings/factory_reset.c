//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"

#include "jolt_gui/jolt_gui.h"
#include "hal/storage/storage.h"
#include "syscore/bg.h"

static const char TAG[] = "jolt_gui/menus/settings/factory_reset.c";

static int factory_reset( jolt_bg_job_t *job ) {
    storage_factory_reset();
    return 0;
}

static void factory_reset_no( lv_obj_t *btn, lv_event_t event ) {
    if( LV_EVENT_SHORT_CLICKED == event) {
        jolt_gui_scr_del();
    }
}

static void factory_reset_yes( lv_obj_t *btn, lv_event_t event ) {
    if( LV_EVENT_SHORT_CLICKED == event) {
        jolt_gui_scr_preloading_create("Factory Reset", "Erasing...");

        esp_err_t err;
        err = jolt_bg_create( factory_reset, NULL, NULL);
    }
}

void menu_factory_reset_create(lv_obj_t *btn, lv_event_t event) {
    if( LV_EVENT_SHORT_CLICKED == event) {
        lv_obj_t *scr = jolt_gui_scr_menu_create(gettext(JOLT_TEXT_FACTORY_RESET));
        jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_NO), factory_reset_no);
        jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_YES), factory_reset_yes);
    }
}

