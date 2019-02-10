#ifndef JOLT_GUI_MENUS_SETTINGS_BLUETOOTH_PAIR_H__
#define JOLT_GUI_MENUS_SETTINGS_BLUETOOTH_PAIR_H__

#include "esp_gap_ble_api.h"
void jolt_gui_bluetooth_pair_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

#endif
