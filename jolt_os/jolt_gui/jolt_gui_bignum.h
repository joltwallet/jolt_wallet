#ifndef JOLT_GUI_BIGNUM_H__
#define JOLT_GUI_BIGNUM_H__

/* n_digits can be set to -1 for no leading zeros */
lv_obj_t *jolt_gui_scr_bignum_create(const char *title, const char *subtitle, uint32_t n, int8_t n_digits);

#endif
