#ifndef __JOLT_LVGL_GUI_ENTRY_H__
#define __JOLT_LVGL_GUI_ENTRY_H__

#include <stddef.h>
#include "../lvgl/lvgl.h"
#include "jolt_gui.h"


#define JOLT_GUI_BACK // Used to close window, or go to the left during entry
void jolt_gui_back_callback();

//lv_res_t jolt_gui_pin_create( void ); 

/* Creates a numeric entry form with "n" digits and a decimal-point "decimal"
 * digits from the right. If decimal is negative, then no decimal will show.
 * Callback gets called after final roller entry.*/
#define JOLT_GUI_NO_DECIMAL -1
lv_obj_t *jolt_gui_numeric_create( int8_t n, int8_t decimal, const char *title,
        lv_action_t cb ); 


/* New API */
#if USE_LV_GROUP == 0
#error "jolt_gui_num: lv_group is required. Enable it in lv_conf.h (USE_LV_GROUP  1) "
#endif

#ifndef CONFIG_JOLT_GUI_ANIM_DIGIT_MS
    #define CONFIG_JOLT_GUI_ANIM_DIGIT_MS 90
#endif
#ifndef CONFIG_JOLT_GUI_NUMERIC_LEN
    // Max number of rollers
    #define CONFIG_JOLT_GUI_NUMERIC_LEN 8
#endif
#ifndef CONFIG_JOLT_GUI_PIN_LEN
    // note: must be less than or equal to CONFIG_JOLT_GUI_NUMERIC_LEN
    #define CONFIG_JOLT_GUI_PIN_LEN 8
#endif

typedef struct {
    lv_obj_t *rollers[CONFIG_JOLT_GUI_NUMERIC_LEN];
    uint8_t len; // Number of digits
    int8_t pos;
    int8_t decimal; // Position of decimal point from right.
    lv_obj_t *decimal_obj;
    uint8_t spacing; // Distance between rollers
    uint8_t offset; // Distance between first roller and left screen
    lv_action_t back_cb;
    lv_action_t enter_cb;
} jolt_gui_num_ext_t;

lv_obj_t *jolt_gui_num_create(lv_obj_t * par, const lv_obj_t * copy);
void jolt_gui_num_set_len(lv_obj_t *num, uint8_t n);
void jolt_gui_num_set_decimal(lv_obj_t *num, int8_t pos);
void jolt_gui_num_set_enter_action(lv_obj_t *num, lv_action_t cb);
void jolt_gui_num_set_back_action(lv_obj_t *num, lv_action_t cb);
//
lv_obj_t *jolt_gui_scr_num_create(const char *title,
        uint8_t len, int8_t dp, lv_action_t cb);
/* Get the numeric object from the screen */
lv_obj_t *jolt_gui_scr_num_get(lv_obj_t *parent);

/* Populates arr with the roller values */
uint8_t jolt_gui_num_get_arr(lv_obj_t *num, uint8_t *arr, uint8_t arr_len);

/* Computes a 256-bit blake2b hash into *hash */
uint8_t jolt_gui_num_get_hash(lv_obj_t *num, uint8_t *hash);


#endif
