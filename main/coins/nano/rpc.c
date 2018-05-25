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
#include "confirmation.h"
#include "contacts.h"

static const char* TAG = "vault_nano";


static vault_rpc_response_t rpc_nano_contact_update(vault_t *vault, vault_rpc_t *cmd, menu8g2_t *menu){
    vault_rpc_response_t response;

    if( nano_confirm_contact_update(menu,
            cmd->nano_contact_update.name, 
            cmd->nano_contact_update.public,
            cmd->nano_contact_update.index) ){
        if( !nano_set_contact_public(cmd->nano_contact_update.public,
                    cmd->nano_contact_update.index) ){
            response = RPC_FAILURE;
        }
        else if( !nano_set_contact_name(cmd->nano_contact_update.name,
                    cmd->nano_contact_update.index) ){
            nano_erase_contact(cmd->nano_contact_update.index);
            response = RPC_FAILURE;
        }
        else{
            response = RPC_SUCCESS;
        }
    }
    else {
        response = RPC_FAILURE;
    }
    return response;
}

static vault_rpc_response_t rpc_nano_public_key(vault_t *vault, vault_rpc_t *cmd, menu8g2_t *menu){
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

static vault_rpc_response_t rpc_nano_block_sign(vault_t *vault, vault_rpc_t *cmd, menu8g2_t *menu){
    vault_rpc_response_t response;

    CONFIDENTIAL uint256_t private_key;
    uint256_t public_key;
    nl_master_seed_to_nano_private_key(private_key, 
            vault->master_seed, cmd->nano_block_sign.index);
    nl_private_to_public(public_key, private_key);

    // Compare block's account with public key
    if( 0 != memcmp(public_key, cmd->nano_block_sign.block.account, BIN_256) ){
        response = RPC_FAILURE;
        goto exit;
    }

    // Prompt user to confirm sign
    if( nano_confirm_block(menu,
                &(cmd->nano_block_sign.frontier),
                &(cmd->nano_block_sign.block)) ){
        nl_block_sign(&(cmd->nano_block_sign.block), private_key);
        response = RPC_SUCCESS;
    }
    else{
        response = RPC_FAILURE;
    }

    exit:
        sodium_memzero(private_key, sizeof(private_key));
        return response;
}

vault_rpc_response_t rpc_nano(vault_t *vault, vault_rpc_t *cmd, menu8g2_t *menu){
    vault_rpc_response_t response;
    switch(cmd->type){
        case(NANO_BLOCK_SIGN):
            ESP_LOGI(TAG, "Executing NANO_BLOCK_SIGN RPC.");
            response = rpc_nano_block_sign(vault, cmd, menu);
            break;
        case(NANO_PUBLIC_KEY):
            ESP_LOGI(TAG, "Executing NANO_PUBLIC_KEY RPC.");
            response = rpc_nano_public_key(vault, cmd, menu);
            break;
        case(NANO_CONTACT_UPDATE):
            ESP_LOGI(TAG, "Executing NANO_CONTACT_UPDATE RPC.");
            response = rpc_nano_contact_update(vault, cmd, menu);
            break;
        default:
            response = RPC_UNDEFINED;
            break;
    }
    return response;
}

