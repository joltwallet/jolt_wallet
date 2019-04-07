/**
 * @file jolt_gui_digit_entry.h
 * @brief Enter a number via dpad
 * @author Brian Pugh
 */

#ifndef __JOLT_LVGL_GUI_DIGIT_ENTRY_H__
#define __JOLT_LVGL_GUI_DIGIT_ENTRY_H__

#include <stddef.h>
#include "lvgl/lvgl.h"

#define JOLT_GUI_SCR_DIGIT_ENTRY_NO_DECIMAL -1
#ifndef CONFIG_JOLT_GUI_SCR_DIGIT_ENTRY_ANIM_DIGIT_MS
    #define CONFIG_JOLT_GUI_SCR_DIGIT_ENTRY_ANIM_DIGIT_MS 90
#endif
#ifndef CONFIG_JOLT_GUI_SCR_DIGIT_ENTRY_MAX_LEN
    // Max number of rollers
    #define CONFIG_JOLT_GUI_SCR_DIGIT_ENTRY_MAX_LEN 8
#endif
#ifndef CONFIG_JOLT_GUI_PIN_LEN
    // note: must be less than or equal to CONFIG_JOLT_GUI_NUMERIC_LEN
    #define CONFIG_JOLT_GUI_PIN_LEN 8
#endif

/**
 * @brief Create a digit-entry screen.
 *
 * By default, the left-most digit is focussed.
 *
 * @param[in] title title-bar string
 * @param[in] n Number of digits to display
 * @param[in] pos decimal point position from the right. 0 for right most, 1 to display tenth's place, etc. -1 for no decimal point.
 * @return screen
 */
lv_obj_t *jolt_gui_scr_digit_entry_create(const char *title,
        int8_t n, int8_t pos);


/*****************
 * Value getters *
 *****************/

/**
 * @brief Get current digits as an array.
 *
 * Decimal position has no impact on this function.
 *
 * @param[in] parent digit-entry screen
 * @param[out] arr array buffer to populate
 * @param[in] arr_len length of provided array buffer
 * @return number of elements (digits) returned. Returns -1 if the provided buffer is too small. 
 */
int8_t jolt_gui_scr_digit_entry_get_arr(lv_obj_t *parent, uint8_t *arr, uint8_t arr_len);

/**
 * @brief Get the 256-bit Blake2b hash of the current value
 * @param[in] parent digit-entry screen
 * @param[out] pointer to a 256-bit buffer
 * @return 0 on success, -1 on failure.
 */
uint8_t jolt_gui_scr_digit_entry_get_hash(lv_obj_t *parent, uint8_t *hash);

/**
 * @brief Get entered value as a double, taking into account the decimal point.
 *
 * WARNING: Primarily used for display purposes only, not financial computation.
 * Floats/Doubles may not be the precise value specified.
 * Exercise heavy caution if using the returned value in signed transactions.
 *
 * @param[in] parent digit-entry screen
 * @return entered value
 */
double jolt_gui_scr_digit_entry_get_double(lv_obj_t *parent);

/**
 * @brief Get the entered value as an integer, decimal point has no impact on this function.
 *
 * All entered values are concatenated.
 *
 * @param[in] parent digit-entry screen
 * @return entered value.
 */
uint32_t jolt_gui_scr_digit_entry_get_int(lv_obj_t *parent);

/******************
 * Value Setters *
 ******************/

/**
 * @brief Sets the currently focused digit pos digits from the right. 0 for the right most digit.
 * @param[in] parent digit-entry screen
 * @param[in] pos 0-indexed entry position from the right to focus on
 */
void jolt_gui_scr_digit_entry_set_pos(lv_obj_t *parent, int8_t pos);

#endif
