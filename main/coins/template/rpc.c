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

#include "../../vault.h"
#include "rpc.h"

static const char* TAG = "vault_temp";

static vault_rpc_response_t rpc_temp_msg_sign(vault_t *vault, vault_rpc_t *cmd){
    /* Write signing code here */
    return RPC_SUCCESS;
}

vault_rpc_response_t rpc_nano(vault_t *vault, vault_rpc_t *cmd){
    vault_rpc_response_t response;
    /* COIN RPC SWITCH */
    switch(cmd->type){
        case(TEMP_MSG_SIGN):
            ESP_LOGI(TAG, "Executing TEMP_MSG_SIGN RPC.");
            response = rpc_temp_msg_sign(vault, cmd);
            break;
        default:
            response = RPC_UNDEFINED;
            break;
    }
    return response;
}

