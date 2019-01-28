/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */


#ifndef __INCLUDE_WIFI_H__
#define __INCLUDE_WIFI_H__

#include "esp_event_loop.h"

void wifi_connect();
esp_err_t wifi_event_handler(void *ctx, system_event_t *event);
void get_ap_info(char * ssid_info, size_t size);

#endif
