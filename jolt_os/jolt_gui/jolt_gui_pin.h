#ifndef __JOLT_GUI_PIN_H__
#define __JOLT_GUI_PIN_H__

#include "lvgl/lvgl.h"

/* Creates a PIN Entry screen.
 *
 * Executes success_cb if the user enters the correct PIN.
 * Executes failure_cb if the user enters an incorrect PIN. 
 *
 * Will always delete itself.
 */

/* On success, populates jolt_gui_store.tmp.mnemonic_bin and calls success_cb.
 *
 */
lv_obj_t *jolt_gui_scr_pin_create(lv_action_t failure_cb, lv_action_t success_cb);

#endif
