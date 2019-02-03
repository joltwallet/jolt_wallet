#include "jolt_gui/jolt_gui.h"


lv_res_t menu_bluetooth_pair_create(lv_obj_t *btn) {
    jolt_gui_scr_bignum_create(gettext(JOLT_TEXT_PAIR), "Placeholder Pairing Code", 123456, -1);
    return LV_RES_OK;
}
