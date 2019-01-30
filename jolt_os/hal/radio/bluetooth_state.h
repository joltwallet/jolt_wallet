#ifndef JOLT_HAL_BLUETOOTH_STATE_H__
#define JOLT_HAL_BLUETOOTH_STATE_H__

#include "stdint.h"
#include "bluetooth_cfg.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

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

extern gatts_profile_inst_t spp_profile_tab[SPP_PROFILE_NUM];

extern uint16_t spp_handle_table[SPP_IDX_NB];

extern xQueueHandle ble_in_queue;

#endif
