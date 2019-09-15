/**
 * @file bluetooth_gatts_profile_a.h
 * @brief GATTS event handler for Profile A
 * @author Brian Pugh
 */

#ifndef JOLT_HAL_BLUETOOTH_GATTS_PROFILE_A_H__
#define JOLT_HAL_BLUETOOTH_GATTS_PROFILE_A_H__

#include "sdkconfig.h"

#if CONFIG_BT_ENABLED
    #include "esp_gap_ble_api.h"
    #include "esp_gatts_api.h"

/**
 * @brief GATTS event handler for Profile A
 */
void gatts_profile_a_event_handler( esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                    esp_ble_gatts_cb_param_t *param );

/**
 * @brief Checks if a client is connected and using Profile A
 * @return True if client is connected; false otherwise
 */
bool gatts_profile_a_is_connected();

#endif

#endif
