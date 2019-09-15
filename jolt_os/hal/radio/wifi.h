/**
 * @file wifi.h
 * @brief High level wifi control functions
 * @author Brian Pugh
 * @bugs
 *     * WiFi credentials not properly updated until a hard reboot.
 */

/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef JOLT_HAL_RADIO_WIFI_H__
#define JOLT_HAL_RADIO_WIFI_H__

#include "esp_event.h"

#define JOLT_WIFI_SSID_MAX_LEN 31  // Not including NULL terminator
#define JOLT_WIFI_PASS_MAX_LEN 63  // Not including NULL terminator

#define JOLT_WIFI_IP_MAX_LEN 45  // Not including NULL terminator

/**
 * @brief Start WiFi
 * @return ESP_OK on success; otherwise the error code of whatever component failed.
 */
esp_err_t jolt_wifi_start();

/**
 * @brief Stop WiFi
 * @return ESP_OK on success; otherwise the error code of whatever component failed.
 */
esp_err_t jolt_wifi_stop();

/**
 * @brief WiFi event handler that needs to be registered with the system.
 */
void jolt_wifi_event_handler( void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data );

/**
 * @brief Get the current IP address.
 * @return Pointer to allocated string. Must be freed. Returns NULL on error.
 */
char* jolt_wifi_get_ip();

/**
 * @brief Get RSSI
 * @return RSSI strength. 0 on error.
 */
int8_t jolt_wifi_get_rssi();

/**
 * @brief Get the currently connected SSID.
 * @return Pointer to allocated string. Must be freed. Returns NULL on error.
 */
char* jolt_wifi_get_ssid();

/**
 * @brief Return whether or not WiFi is currently enabled.
 * @return `true` if wifi is enabled; `false` otherwise.
 */
bool jolt_wifi_get_en();

#endif
