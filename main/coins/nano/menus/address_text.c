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

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sodium.h"
#include "esp_log.h"

#include "nano_lib.h"

#include "menu8g2.h"
#include "submenus.h"
#include "../../../globals.h"
#include "../../../vault.h"
#include "../../../gui/statusbar.h"

static const char* TAG = "address_text";
static const char TITLE[] = "Nano Address";

void menu_nano_address_text(menu8g2_t *prev){
    vault_rpc_t rpc;
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);

    nvs_handle nvs_h;
    init_nvm_namespace(&nvs_h, "nano");
    if(ESP_OK != nvs_get_u32(nvs_h, "index", &(rpc.nano_public_key.index))){
        rpc.nano_public_key.index = 0;
    }
    nvs_close(nvs_h);

    rpc.type = NANO_PUBLIC_KEY;

    if(vault_rpc(&rpc) != RPC_SUCCESS){
        goto exit;
    }

    char address[ADDRESS_BUF_LEN];
    nl_public_to_address(address, sizeof(address),
            rpc.nano_public_key.block.account);

    ESP_LOGI(TAG, "Address: %s", address);

    for(;;){
        if(menu8g2_display_text_title(&menu, address, TITLE)
                & (1ULL << EASY_INPUT_BACK)){
            goto exit;
        }
    }

    exit:
        return;
}
