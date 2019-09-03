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
esp_err_t wifi_event_handler(void *ctx, system_event_t *event);

/**
 * @brief Gets SSID, RSSI, and IP address and formats it into a string
 * @param[out] ssid_info Gathered data (NULL-terminated string)
 * @param[in] size size of ssid_info buffer
 */
void get_ap_info(char * ssid_info, size_t size);

#endif
