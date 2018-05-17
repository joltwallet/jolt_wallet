#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_log.h"
#include "sodium.h"

#include "rpc.h"

static const char* TAG = "vault_nano";

static vault_rpc_response_t rpc_nano_public_key(vault_t *vault, vault_rpc_t *cmd){
    // Derive private key from mnemonic
    vault_rpc_response_t response;
    CONFIDENTIAL uint256_t private_key;
    nl_master_seed_to_nano_private_key(private_key, 
            vault->master_seed,
            cmd->public_key.index);
    // Derive public key from private
    nl_private_to_public(cmd->public_key.block.account, private_key);
    sodium_memzero(private_key, sizeof(private_key));
    response = RPC_SUCCESS;
    return response;
}

static vault_rpc_response_t rpc_nano_block_sign(vault_t *vault, vault_rpc_t *cmd){
    vault_rpc_response_t response;

    // todo: prompt user to confirm signing

    CONFIDENTIAL uint256_t private_key;
    nl_master_seed_to_nano_private_key(private_key, 
            vault->master_seed,
            cmd->public_key.index);

    nl_block_sign(&(cmd->block_sign.block), private_key);

    sodium_memzero(private_key, sizeof(private_key));
    response = RPC_SUCCESS;
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
    }
    return response;
}

