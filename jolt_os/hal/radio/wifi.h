/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */


#ifndef __INCLUDE_WIFI_H__
#define __INCLUDE_WIFI_H__

void wifi_connect();
void get_ap_info(char * ssid_info, size_t size);
bool set_wifi_credentials(char *ssid, char *pass);

#endif
