/**
 * @file jolt_gui_bignum.h
 * @brief Display a number in large-font
 * @author Brian Pugh
 */

#ifndef JOLT_GUI_BIGNUM_H__
#define JOLT_GUI_BIGNUM_H__

/* n_digits can be set to -1 for no leading zeros */
/**
 * @brief Display an integer in large font.
 * @param[in] title title-bar string
 * @param[in] subtitle string to diplsay above the large-font number
 * @param[in] n integer to display in large font
 * @param[in] n_digits number of digits to display. Set to -1 for no-leading zeros
 * @return screen
 */
jolt_gui_obj_t *jolt_gui_scr_bignum_create( const char *title, const char *subtitle, uint32_t n, int8_t n_digits );

#endif
