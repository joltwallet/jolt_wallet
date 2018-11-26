#ifndef JOLT_HAL_RADIO_BLE_SPP_RECV_BUF_H__
#define JOLT_HAL_RADIO_BLE_SPP_RECV_BUF_H__

#include "esp_gatts_api.h"

bool store_wr_buffer(esp_ble_gatts_cb_param_t *p_data);
void free_write_buffer(void);
void print_write_buffer(void);
char *consolidate_write_buffer(void);

#endif
