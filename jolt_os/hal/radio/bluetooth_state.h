/**
 * @file bluetooth_state.h
 * @brief Variables that define the current state of Bluetooth
 * @author Brian Pugh
 */

#ifndef JOLT_HAL_BLUETOOTH_STATE_H__
#define JOLT_HAL_BLUETOOTH_STATE_H__

#include "sdkconfig.h"

#if CONFIG_BT_ENABLED

#include "stdint.h"
#include "bluetooth_cfg.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/**
 * @brief GATTS profile object
 */
typedef struct gatts_profile_inst_t {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
} gatts_profile_inst_t;

/**
 * @brief Table of available profiles
 */
extern gatts_profile_inst_t spp_profile_tab[SPP_PROFILE_NUM];

/**
 * @brief Attribute Handle Table
 */
extern uint16_t spp_handle_table[SPP_IDX_NB];

/**
 * @brief Queue to shuttle data from the event handler to ble_in_task
 */
extern xQueueHandle ble_in_queue;

/**
 * @brief True if currently in pairing mode, false otherwise
 */
extern bool jolt_bluetooth_pair_mode;

#endif

#endif
