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

static const char* TAG = "vault_nano";

static vault_rpc_response_t rpc_nano_public_key(vault_t *vault, vault_rpc_t *cmd){
    // Derive private key from mnemonic
    vault_rpc_response_t response;
    CONFIDENTIAL uint256_t private_key;
    nl_master_seed_to_nano_private_key(private_key, 
            vault->master_seed,
            cmd->nano_public_key.index);
    // Derive public key from private
    nl_private_to_public(cmd->nano_public_key.block.account, private_key);
    sodium_memzero(private_key, sizeof(private_key));
    response = RPC_SUCCESS;
    return response;
}

static vault_rpc_response_t rpc_nano_block_sign(vault_t *vault, vault_rpc_t *cmd){
    vault_rpc_response_t response;

    uint32_t index;
    nvs_handle nvs_h;
    init_nvm_namespace(&nvs_h, "nano");
    if(ESP_OK != nvs_get_u32(nvs_h, "index", &(index))){
        index = 0;
    }
    nvs_close(nvs_h);

    CONFIDENTIAL uint256_t private_key;
    uint256_t public_key;
    nl_master_seed_to_nano_private_key(private_key, 
            vault->master_seed, index);
    nl_private_to_public(public_key, private_key);

    // Compare block's account with public key
    if( 0 != memcmp(public_key, cmd->nano_block_sign.block.account, BIN_256) ){
        response = RPC_FAILURE;
        goto exit;
    }

    //todo prompt user here
    
    nl_block_sign(&(cmd->nano_block_sign.block), private_key);
    response = RPC_SUCCESS;

    exit:
        sodium_memzero(private_key, sizeof(private_key));
        return response;
}

vault_rpc_response_t rpc_nano(vault_t *vault, vault_rpc_t *cmd){
    vault_rpc_response_t response;
    switch(cmd->type){
        case(NANO_BLOCK_SIGN):
            ESP_LOGI(TAG, "Executing NANO_BLOCK_SIGN RPC.");
            response = rpc_nano_block_sign(vault, cmd);
            break;
        case(NANO_PUBLIC_KEY):
            ESP_LOGI(TAG, "Executing NANO_PUBLIC_KEY RPC.");
            response = rpc_nano_public_key(vault, cmd);
            break;
        default:
            response = RPC_UNDEFINED;
            break;
    }
    return response;
}

