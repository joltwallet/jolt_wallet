#include "jolt_gui/jolt_gui.h"


lv_res_t menu_bluetooth_unbond_create(lv_obj_t *btn) {
    jolt_gui_scr_text_create(gettext(JOLT_TEXT_UNBONDS), "Placeholder Text");
    return LV_RES_OK;
}
