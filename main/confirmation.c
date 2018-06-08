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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_log.h"
#include "sodium.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "menu8g2.h"
#include "nano_lib.h"

#include "vault.h"
#include "globals.h"

static const char *TAG = "nano_conf";


bool menu_confirm_action(menu8g2_t *menu, char *buf){
    /* Draws screen for yes/no action */
    uint64_t button;
    for(;;){
        button = menu8g2_display_text_title(menu, buf, "Confirm Action");
        if((1ULL << EASY_INPUT_BACK) & button){
            return false;
        }
        else if((1ULL << EASY_INPUT_ENTER) & button){
            return true;
        }
    }
}

bool syscore_confirm_wifi_update(menu8g2_t *prev_menu, const char *ssid, const char *pass){
    menu8g2_t menu;
    menu8g2_copy(&menu, prev_menu);

    bool response;

    CONFIDENTIAL char buf[200];
    snprintf(buf, sizeof(buf), "Update WiFi?:\nSSID: %s\nPass: %s",
            ssid, pass);

    if ( !menu_confirm_action(&menu, buf) ){
        response = false;
        goto exit;
    }
    else {
        response = true;
        goto exit;
    }

    exit:
        sodium_memzero(buf, sizeof(buf));
        return response;
}

