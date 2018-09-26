/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_CONFIRMATION_H__
#define __JOLT_CONFIRMATION_H__

#include "menu8g2.h"

bool menu_confirm_action(menu8g2_t *menu, char *buf);
bool syscore_confirm_wifi_update(menu8g2_t *prev_menu, const char *ssid, const char *pass);

#endif
