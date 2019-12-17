/**
 * @file set_wifi_credentials.h
 * @brief primary function to configure wifi credentials
 * @author Brian Pugh
 */

#ifndef JOLT_SYSCORE_SET_WIFI_CREDENTIALS_H__
#define JOLT_SYSCORE_SET_WIFI_CREDENTIALS_H__

#include "stdbool.h"

/**
 * @brief Update saved WiFi credentials.
 *
 * Will prompt user for PIN.
 *
 * Must be called from CLI.
 *
 * Is non-blocking.
 *
 * @param[in] ssid NULL-terminated network SSID to save
 * @param[in] pass NULL-terminated network password
 * @param[in] free_vars If set to `true`, `ssid` and `pass` will be freed upon
 *    exit.
 */
void set_wifi_credentials( const char *ssid, const char *pass, bool free_vars );

#endif
