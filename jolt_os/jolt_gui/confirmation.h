/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_CONFIRMATION_H__
#define __JOLT_CONFIRMATION_H__


bool menu_confirm_action(char *buf);
bool syscore_confirm_wifi_update(const char *ssid, const char *pass);

#endif
