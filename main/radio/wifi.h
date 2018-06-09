/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */


#ifndef __INCLUDE_WIFI_H__
#define __INCLUDE_WIFI_H__

// The identity of the access point to which we wish to connect.
//#define AP_TARGET_SSID     ""

// The password we need to supply to the access point for authorization.
//#define AP_TARGET_PASSWORD ""

void wifi_connect();
void get_ap_info(char * ssid_info, size_t size);
void wifi_task(void *pvParameters);
uint8_t get_wifi_strength();

#endif
