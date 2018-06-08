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

#include "helpers.h"
#include "vault.h"
#include "rpc_syscore.h"
#include "confirmation.h"

static const char* TAG = "vault_syscore";

static vault_rpc_response_t rpc_syscore_wifi_update(vault_t *vault,
        vault_rpc_t *cmd, menu8g2_t *menu){
    vault_rpc_response_t response;
    if( syscore_confirm_wifi_update(menu,
                cmd->syscore_wifi_update.ssid,
                cmd->syscore_wifi_update.pass) ){
        nvs_handle h;
        init_nvm_namespace(&h, "user");
        nvs_set_str(h, "wifi_ssid", cmd->syscore_wifi_update.ssid);
        nvs_set_str(h, "wifi_pass", cmd->syscore_wifi_update.pass);
        esp_err_t err = nvs_commit(h);
        nvs_close(h);

        response = RPC_SUCCESS;
    }
    else {
        response = RPC_CANCELLED;
    }

    return response;
}

vault_rpc_response_t rpc_syscore(vault_t *vault, vault_rpc_t *cmd, menu8g2_t *menu){
    vault_rpc_response_t response;
    switch(cmd->type){
        case(SYSCORE_WIFI_UPDATE):
            ESP_LOGI(TAG, "Executing SYSCORE_WIFI_UPDATE RPC.");
            response = rpc_syscore_wifi_update(vault, cmd, menu);
            break;
        default:
            response = RPC_UNDEFINED;
            break;
    }
    return response;
}

