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

static const char* TAG = "vault_syscore";


vault_rpc_response_t rpc_syscore(vault_t *vault, vault_rpc_t *cmd, menu8g2_t *menu){
    vault_rpc_response_t response;
    switch(cmd->type){
        default:
            response = RPC_UNDEFINED;
            break;
    }
    return response;
}

