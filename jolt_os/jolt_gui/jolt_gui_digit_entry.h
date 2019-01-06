#ifndef __JOLT_LVGL_GUI_DIGIT_ENTRY_H__
#define __JOLT_LVGL_GUI_DIGIT_ENTRY_H__

#include <stddef.h>
#include "../lvgl/lvgl.h"

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

lv_obj_t *jolt_gui_scr_digit_entry_create(const char *title,
        int8_t n, int8_t pos);

/* Value getters */
int8_t jolt_gui_scr_digit_entry_get_arr(lv_obj_t *parent, uint8_t *arr, uint8_t arr_len);
uint8_t jolt_gui_scr_digit_entry_get_hash(lv_obj_t *parent, uint8_t *hash);


/* Note: the float/double functions are primarily used for display purposes. 
 * Floats/Doubles are not precisely the value specified.
 * Exercise heavy caution if using the returned value in signed transactions.*/

/* Returns the entered value as a double, taking into account the decimal point.*/
double jolt_gui_scr_digit_entry_get_double(lv_obj_t *parent);

/* Return the enter value as all the rollers concatenated.
 * Does not take into account decimal place. */
uint32_t jolt_gui_scr_digit_entry_get_int(lv_obj_t *parent);

/* Action Setters */
void jolt_gui_scr_digit_entry_set_back_action(lv_obj_t *parent, lv_action_t cb);
void jolt_gui_scr_digit_entry_set_enter_action(lv_obj_t *parent, lv_action_t cb);

#endif
