#include "jolt_gui/jolt_gui.h"
#include "hal/storage/storage.h"
#include "syscore/bg.h"

static int factory_reset( jolt_bg_job_t *job ) {
    storage_factory_reset();
    return 0;
}

static lv_res_t factory_reset_back( lv_obj_t *btn ) {
    jolt_gui_scr_del();
    return LV_RES_INV;
}

static lv_res_t factory_reset_enter( lv_obj_t *btn ) {
    jolt_gui_scr_preloading_create("Factory Reset", "Erasing...");

    esp_err_t err;
    err = jolt_bg_create( factory_reset, NULL, NULL);

    return err;
}

lv_res_t menu_factory_reset_create(lv_obj_t *btn) {
    lv_obj_t *scr = jolt_gui_scr_menu_create(gettext(JOLT_TEXT_FACTORY_RESET));
    jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_NO), factory_reset_back);
    jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_YES), factory_reset_enter);
    return LV_RES_OK;
}

