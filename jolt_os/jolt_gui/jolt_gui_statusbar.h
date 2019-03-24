/**
 * @file jolt_gui_statusbar.h
 * @brief Statusbar indicators
 * @author Brian Pugh
 */

#ifndef __JOLT_GUI_STATUSBAR_H__
#define __JOLT_GUI_STATUSBAR_H__

#include "lvgl/lvgl.h"

#ifndef CONFIG_JOLT_GUI_STATUSBAR_H
    #define CONFIG_JOLT_GUI_STATUSBAR_H 12
#endif

/**
 * @brief Initializes and periodically updates the statusbar
 *
 * Creates all the necessary LVGL objects and creates an lv_task to periodically
 * update the statusbar based on hw_monitors values.
 */
void statusbar_create();

/**
 * @brief Get the statusbar label that displays symbols
 * @return lv_label object
 */
lv_obj_t *statusbar_get_label();

#endif
