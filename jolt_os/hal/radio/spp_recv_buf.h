/**
 * @file spp_recv_buf.h
 * @brief Datastructures for storing intermediate bluetooth communications
 * @author Brian Pugh
 * @bugs
 *     *might not really be used; need to investigate long BLE SPP messages.
 */

#ifndef JOLT_HAL_RADIO_BLE_SPP_RECV_BUF_H__
#define JOLT_HAL_RADIO_BLE_SPP_RECV_BUF_H__

#include "sdkconfig.h"

#if CONFIG_BT_ENABLED
    #include "esp_gatts_api.h"

/**
 * @brief Copy data into the write buffer
 *
 * Allocates memory and stores the data in a singly linked list
 *
 * @param[in] p_data
 * @return True on success, false otherwise
 */
bool store_wr_buffer( esp_ble_gatts_cb_param_t *p_data );

/**
 * @brief free the entire singly linked list buffer
 */
void free_write_buffer( void );

/**
 * @brief Print the current contexts of the buffer to UART console
 */
void print_write_buffer( void );

#endif

#endif
