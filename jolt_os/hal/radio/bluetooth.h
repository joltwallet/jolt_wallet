/**
 * @file bluetooth.h
 * @brief High level bluetooth control
 * @author Brian Pugh
 * @bugs
 *     * select is not fully implemented
 *     * low level whitelisting support is iffy
 */

#ifndef JOLT_BLUETOOTH_H___
#define JOLT_BLUETOOTH_H___

#include <stdint.h>
#include "esp_vfs_dev.h"
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"

#define JOLT_BLE_WHITELIST_ADD       true
#define JOLT_BLE_WHITELIST_REMOVE    false
#define JOLT_BLE_WHITELIST_ADDR_TYPE BLE_WL_ADDR_TYPE_RANDOM

/**
 * Objects that store bluetooth packets
 */
typedef struct {
    uint8_t *data;
    uint32_t len;
#if CONFIG_JOLT_BT_PROFILING
    uint64_t t_receive; /**< System ime in uS the packet was received at */
#endif
} ble_packet_t;

#if CONFIG_JOLT_BT_PROFILING
extern uint64_t ble_packet_cum_life;
extern uint32_t ble_packet_n;
#endif

/**
 * @brief Stream for BLE STDIN
 */
extern FILE *ble_stdin;

/**
 * @brief Stream for BLE STDOUT
 */
extern FILE *ble_stdout;

/**
 * @brief Stream for BLE STDERR
 */
extern FILE *ble_stderr;

/**
 * @brief Register the bluetooth spp driver
 */
void esp_vfs_dev_ble_spp_register();

/**
 * @brief Start bluetooth and related services
 * @return ESP_OK on success; otherwise the error code of whatever component failed.
 */
esp_err_t jolt_bluetooth_start();

/**
 * @brief Stop bluetooth and related services
 * @return ESP_OK on success; otherwise the error code of whatever component failed.
 */
esp_err_t jolt_bluetooth_stop();

/**
 * @brief Start general advertising
 * @return ESP_OK on success; otherwise the error code of whatever component failed.
 */
esp_err_t jolt_bluetooth_adv_all_start();

/**
 * @brief Start advertising to whitelisted devices. Not exactly working.
 * @return ESP_OK on success; otherwise the error code of whatever component failed.
 */
esp_err_t jolt_bluetooth_adv_wht_start();

/**
 * @brief Stop Advertising
 * @return ESP_OK on success; otherwise the error code of whatever component failed.
 */
esp_err_t jolt_bluetooth_adv_stop();

/**
 * @brief Security Config
 * @param[in] bond To bond with client device after pairing
 */
void jolt_bluetooth_config_security( bool bond );

/**
 * @brief Read a character from the bluetooth stream.
 * @param[in] fd File Descriptor; ignored. Just set it to 0.
 * @param[in] timeout; Maximum number of ticks to wait.
 * @return Character read. Returns -1 on timeout.
 */
int ble_read_char( int fd, TickType_t timeout );

/**
 * @brief Read data from bluetooth stream with timeout.
 * @param[in] fd File Descriptor; ignored. Just set it to 0.
 * @param[out] data Data buffer to store output bytes.
 * @param[in] size Number of bytes to read.
 * @param[in] timeout; Maximum number of ticks to wait.
 * @return Number of bytes read.
 */
ssize_t ble_read_timeout( int fd, void *data, size_t size, TickType_t timeout );

/**
 * @brief Set line ending rules.
 * @param[in] type ending rule. -1 for no rules.
 */
void ble_set_rx_line_endings( esp_line_endings_t mode );

#endif
