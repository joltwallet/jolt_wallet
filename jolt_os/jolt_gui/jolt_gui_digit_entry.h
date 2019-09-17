/**
 * @file jolt_gui_digit_entry.h
 * @brief Enter a number via dpad
 * @author Brian Pugh
 */

#ifndef JOLT_GUI_DIGIT_ENTRY_H__
#define JOLT_GUI_DIGIT_ENTRY_H__

#include <stddef.h>
#include "jolt_gui/jolt_gui.h"

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
 * @param[in] pos decimal point position from the right. 0 for right most, 1 to display tenth's place, etc. -1 for no
 * decimal point.
 * @return screen
 */
jolt_gui_obj_t *jolt_gui_scr_digit_entry_create( const char *title, int8_t n, int8_t pos );

/*****************
 * Value getters *
 *****************/

/**
 * @brief Get current digits as an array.
 *
 * Decimal position has no impact on this function.
 *
 * @param[in] digit_entry active digit-entry object
 * @param[out] arr array buffer to populate
 * @param[in] arr_len length of provided array buffer
 * @return number of elements (digits) returned. Returns -1 on error.
 */
int8_t jolt_gui_obj_digit_entry_get_arr( jolt_gui_obj_t *digit_entry, uint8_t *arr, uint8_t arr_len );

/**
 * @brief Get current digits as an array.
 *
 * Decimal position has no impact on this function.
 *
 * @param[in] scr digit-entry screen
 * @param[out] arr array buffer to populate
 * @param[in] arr_len length of provided array buffer
 * @return number of elements (digits) returned. Returns -1 on error.
 */
int8_t jolt_gui_scr_digit_entry_get_arr( jolt_gui_obj_t *scr, uint8_t *arr, uint8_t arr_len );

/**
 * @brief Get the 256-bit Blake2b hash of the current value
 * @param[in] digit_entry active digit-entry object
 * @param[out] pointer to a 256-bit buffer
 * @return 0 on success, 1 on failure.
 */
uint8_t jolt_gui_obj_digit_entry_get_hash( jolt_gui_obj_t *digit_entry, uint8_t *hash );

/**
 * @brief Get the 256-bit Blake2b hash of the current value
 * @param[in] digit_entry digit-entry screen
 * @param[out] pointer to a 256-bit buffer
 * @return 0 on success, 1 on failure.
 */
uint8_t jolt_gui_scr_digit_entry_get_hash( jolt_gui_obj_t *scr, uint8_t *hash );

/**
 * @brief Get entered value as a double, taking into account the decimal point.
 *
 * WARNING: Primarily used for display purposes only, not financial computation.
 * Floats/Doubles may not be the precise value specified.
 * Exercise heavy caution if using the returned value in signed transactions.
 *
 * @param[in] digit_entry active digit-entry object
 * @return entered value. Returns -1 on error.
 */
double jolt_gui_obj_digit_entry_get_double( jolt_gui_obj_t *digit_entry );

/**
 * @brief Get entered value as a double, taking into account the decimal point.
 *
 * WARNING: Primarily used for display purposes only, not financial computation.
 * Floats/Doubles may not be the precise value specified.
 * Exercise heavy caution if using the returned value in signed transactions.
 *
 * @param[in] scr digit_entry screen
 * @return entered value. Returns -1 on error.
 */
double jolt_gui_scr_digit_entry_get_double( jolt_gui_obj_t *scr );

/**
 * @brief Get the entered value as an integer, decimal point has no impact on this function.
 *
 * All entered values are concatenated.
 *
 * @param[in] digit_entry active digit-entry object
 * @return entered value. Returns -1 on error.
 */
int32_t jolt_gui_obj_digit_entry_get_int( jolt_gui_obj_t *digit_entry );

/**
 * @brief Get the entered value as an integer, decimal point has no impact on this function.
 *
 * All entered values are concatenated.
 *
 * @param[in] scr digit-entry object
 * @return entered value. Returns -1 on error.
 */
int32_t jolt_gui_scr_digit_entry_get_int( jolt_gui_obj_t *scr );

/******************
 * Value Setters *
 ******************/

/**
 * @brief Sets the currently focused digit pos digits from the right. 0 for the right most digit.
 * @param[in] scr digit-entry *screen*
 * @param[in] pos 0-indexed entry position from the right to focus on
 */
void jolt_gui_scr_digit_entry_set_pos( jolt_gui_obj_t *scr, int8_t pos );

#endif
