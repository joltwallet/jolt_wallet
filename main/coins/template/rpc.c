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

