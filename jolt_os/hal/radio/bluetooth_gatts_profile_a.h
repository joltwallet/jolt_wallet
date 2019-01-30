#ifndef JOLT_HAL_BLUETOOTH_GATTS_PROFILE_A_H__
#define JOLT_HAL_BLUETOOTH_GATTS_PROFILE_A_H__

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"

void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, 
        esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

bool gatts_profile_a_is_connected();

#endif
