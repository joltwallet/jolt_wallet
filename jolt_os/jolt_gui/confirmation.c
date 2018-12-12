/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#if 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_log.h"
#include "sodium.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "../vault.h"
#include "../globals.h"

#include "../jolt_gui/jolt_gui.h"

static const char *TAG = "confirmation";


bool menu_confirm_action(char *buf){
    /* Draws screen for yes/no action */
    uint64_t button;
    for(;;){
        
        jolt_gui_text_create("Confirm Action", buf);
        //button = menu8g2_display_text_title(menu, buf, "Confirm Action");
        if((1ULL << EASY_INPUT_BACK) & button){
            return false;
        }
        else if((1ULL << EASY_INPUT_ENTER) & button){
            return true;
        }
    }
}

bool syscore_confirm_wifi_update(const char *ssid, const char *pass){
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

#endif
