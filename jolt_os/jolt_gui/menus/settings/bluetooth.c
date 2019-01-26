#include "jolt_gui/jolt_gui.h"

lv_obj_t *sw_en = NULL;


static lv_res_t sw_en_cb(lv_obj_t *btn) {
    lv_sw_toggle_anim(sw_en);
    return LV_RES_OK;
}

lv_res_t menu_bluetooth_create(lv_obj_t *btn) {
    lv_obj_t *scr = jolt_gui_scr_menu_create(gettext(JOLT_TEXT_SETTINGS));
    lv_obj_t *btn_en = jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_BLUETOOTH_ENABLE), sw_en_cb);
    jolt_gui_scr_menu_add(scr, NULL, "test", NULL);
    jolt_gui_scr_menu_add(scr, NULL, "test", NULL);
    jolt_gui_scr_menu_add(scr, NULL, "test", NULL);
    jolt_gui_scr_menu_add(scr, NULL, "test", NULL);
    jolt_gui_scr_menu_add(scr, NULL, "test", NULL);
    jolt_gui_scr_menu_add(scr, NULL, "test", NULL);
    jolt_gui_scr_menu_add(scr, NULL, "test", NULL);

    /* Add Switch Element */
    sw_en = jolt_gui_scr_menu_add_sw( btn_en );

    return LV_RES_OK;
}
