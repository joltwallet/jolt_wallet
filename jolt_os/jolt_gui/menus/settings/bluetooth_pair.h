/**
 * @file bluetooth_pair.h
 * @brief GUI elements for pairing a bluetooth device
 * @author Brian Pugh
 */

#ifndef JOLT_GUI_MENUS_SETTINGS_BLUETOOTH_PAIR_H__
#define JOLT_GUI_MENUS_SETTINGS_BLUETOOTH_PAIR_H__

#include "sdkconfig.h"

#if CONFIG_BT_ENABLED

#include "esp_gap_ble_api.h"

/**
 * @brief Callback that needs to be included in the GAP event loop handler.
 */
void jolt_gui_bluetooth_pair_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

#endif

#endif
