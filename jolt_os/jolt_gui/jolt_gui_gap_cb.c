#include "jolt_gui.h"
#include "jolt_gui/menus/settings/bluetooth_pair.h"

void jolt_gui_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    jolt_gui_bluetooth_pair_gap_cb(event, param);
}
