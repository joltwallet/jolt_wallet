/**
 * @file set_wifi_credentials.h
 * @brief primary function to configure wifi credentials
 * @author Brian Pugh
 */

#ifndef JOLT_SYSCORE_SET_WIFI_CREDENTIALS_H__
#define JOLT_SYSCORE_SET_WIFI_CREDENTIALS_H__

/**
 * @brief Update saved WiFi credentials.
 *
 * Will prompt user for PIN.
 *
 * @param[in] ssid NULL-terminated network SSID to save
 * @param[in] pass NULL-terminated network password
 */
void set_wifi_credentials(const char *ssid, const char *pass);

#endif
