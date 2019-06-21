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

#include "sdkconfig.h"

#define JOLT_BLE_WHITELIST_ADD    true
#define JOLT_BLE_WHITELIST_REMOVE false
#define JOLT_BLE_WHITELIST_ADDR_TYPE BLE_WL_ADDR_TYPE_RANDOM

#if CONFIG_BT_ENABLED
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

#endif

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
void jolt_bluetooth_config_security(bool bond);

int ble_read_char(int fd, TickType_t timeout);

ssize_t ble_read_timeout(int fd, void* data, size_t size, TickType_t timeout);
#endif
