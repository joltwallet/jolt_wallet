/**
 * @file bluetooth_pair.h
 * @brief GUI elements for pairing a bluetooth device
 * @author Brian Pugh
 */

#ifndef JOLT_GUI_MENUS_SETTINGS_BLUETOOTH_PAIR_H__
#define JOLT_GUI_MENUS_SETTINGS_BLUETOOTH_PAIR_H__

#include "sdkconfig.h"

#if CONFIG_BT_ENABLED

/**
 * @brief Callback that needs to be included in the GAP event loop handler.
 */
int jolt_gui_bluetooth_pair_gap_cb( struct ble_gap_event *event, void *arg );

#endif

#endif
