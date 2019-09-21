/**
 * @file jolt_gui_statusbar.h
 * @brief Statusbar indicators
 * @author Brian Pugh
 */

#ifndef JOLT_GUI_STATUSBAR_H__
#define JOLT_GUI_STATUSBAR_H__

#include "jolt_gui/jolt_gui.h"

#ifndef CONFIG_JOLT_GUI_STATUSBAR_H
    #define CONFIG_JOLT_GUI_STATUSBAR_H 12
#endif

enum {
    JOLT_GUI_STATUSBAR_ICON_BATTERY_IDX,
    JOLT_GUI_STATUSBAR_ICON_CHIP_IDX,
    JOLT_GUI_STATUSBAR_ICON_BLUETOOTH_IDX,
    JOLT_GUI_STATUSBAR_ICON_WIFI_IDX,
    JOLT_GUI_STATUSBAR_ICON_LOCK_IDX,
};
#define JOLT_GUI_STATUSBAR_ICON_BATTERY   ( 1 << JOLT_GUI_STATUSBAR_ICON_BATTERY_IDX )
#define JOLT_GUI_STATUSBAR_ICON_CHIP      ( 1 << JOLT_GUI_STATUSBAR_ICON_CHIP_IDX )
#define JOLT_GUI_STATUSBAR_ICON_BLUETOOTH ( 1 << JOLT_GUI_STATUSBAR_ICON_BLUETOOTH_IDX )
#define JOLT_GUI_STATUSBAR_ICON_WIFI      ( 1 << JOLT_GUI_STATUSBAR_ICON_WIFI_IDX )
#define JOLT_GUI_STATUSBAR_ICON_LOCK      ( 1 << JOLT_GUI_STATUSBAR_ICON_LOCK_IDX )

typedef uint8_t jolt_gui_statusbar_icon_t;

/**
 * @brief Initializes and periodically updates the statusbar
 *
 * Creates all the necessary LVGL objects and creates an lv_task to periodically
 * update the statusbar based on hw_monitors values.
 */
void jolt_gui_statusbar_create();

/**
 * @brief Get the statusbar label that displays symbols
 * @return lv_label object
 */
jolt_gui_obj_t *jolt_gui_statusbar_get_label();

/**
 * @brief Set which statusbar icons will be shown.
 * @param[in] flags OR'd options from `jolt_gui_statusbar_icon_t`;
 */
void jolt_gui_statusbar_set_icons( uint32_t flags );

#endif
