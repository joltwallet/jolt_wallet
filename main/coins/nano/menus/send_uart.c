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
#include <string.h>
#include "esp_log.h"

#include "nano_lib.h"

#include "menu8g2.h"
#include "../../../vault.h"
#include "submenus.h"
#include "../../../globals.h"
#include "../../../loading.h"
#include "../confirmation.h"
#include "../../../gui.h"
#include "../../../uart.h"

#include "nano_lws.h"
#include "nano_parse.h"

static const char TAG[] = "nano_send";
static const char TITLE[] = "Send Nano";


void menu_nano_send_uart(menu8g2_t *prev){
    /*
     * Blocks involved:
     * new_block - block we are creating
     * prev_block - frontier of our account chain
     */
    
    vault_rpc_t rpc;
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);

    /**************************************
     * Get Destination Address and Amount *
     **************************************/
    loading_enable();
    loading_text_title("Enter Send Details", TITLE);
    
    char dest_address[ADDRESS_BUF_LEN];
    esp_log_level_set("*", ESP_LOG_ERROR);
    flush_uart();
    printf("\nEnter a destination address: ");
    get_serial_input(dest_address, sizeof(dest_address));
    esp_log_level_set("*", CONFIG_LOG_DEFAULT_LEVEL);

    // Verify Destination Address
    uint256_t dest_public_key;
    if(E_SUCCESS != nl_address_to_public(dest_public_key, dest_address)){
        loading_disable();
        menu8g2_display_text_title(&menu, "Invalid Address", TITLE); \
        ESP_LOGE(TAG, "\nInvalid Address %s\n", dest_address); \
        goto exit;
    }

    esp_log_level_set("*", ESP_LOG_ERROR);
    flush_uart();
    
    char dest_amount_buf[40];
    printf("\nEnter amount (raw): ");
    get_serial_input_int(dest_amount_buf, sizeof(dest_amount_buf)); //This only accepts numbers
    esp_log_level_set("*", CONFIG_LOG_DEFAULT_LEVEL);
    
    mbedtls_mpi transaction_amount;
    mbedtls_mpi_init(&transaction_amount);
    mbedtls_mpi_read_string(&transaction_amount, 10, dest_amount_buf);
    
    /******************
     * Get My Address *
     ******************/
    uint32_t index;
    nvs_handle nvs_h;
    init_nvm_namespace(&nvs_h, "nano");
    if(ESP_OK != nvs_get_u32(nvs_h, "index", &(index))){
        index = 0;
    }
    nvs_close(nvs_h);
    rpc.nano_public_key.index = index;

    rpc.type = NANO_PUBLIC_KEY;
    loading_disable();
    if(vault_rpc(&rpc) != RPC_SUCCESS){
        goto exit;
    }
    loading_enable();
    uint256_t my_public_key;
    memcpy(my_public_key, rpc.nano_public_key.block.account, sizeof(my_public_key));

    char my_address[ADDRESS_BUF_LEN];
    nl_public_to_address(my_address, sizeof(my_address), my_public_key);
    
    ESP_LOGI(TAG, "My Address: %s\n", my_address);

    /***********************************
     * Get My Account's Frontier Block *
     ***********************************/
    // Assumes State Blocks Only
    // Outcome:
    //     * frontier_hash, frontier_block
    //loading_enable();
    loading_text_title("Connecting", TITLE);
    
    hex256_t frontier_hash;
    nl_block_t frontier_block;
    nl_block_init(&frontier_block);
    memcpy(frontier_block.account, my_public_key, BIN_256);
    uint64_t proof_of_work;

    switch( nanoparse_lws_frontier_block(&frontier_block) ){
        case E_SUCCESS:
            ESP_LOGI(TAG, "Frontier Block Found");
            uint256_t frontier_hash_bin;
            ESP_ERROR_CHECK(nl_block_compute_hash(&frontier_block, frontier_hash_bin));
            sodium_bin2hex(frontier_hash, sizeof(frontier_hash),
                    frontier_hash_bin, sizeof(frontier_hash_bin));
            break;
        default:
            //To send requires a previous Open Block
            ESP_LOGI(TAG, "Couldn't fetch frontier.");
            loading_disable();
            menu8g2_display_text_title(&menu, "Could not fetch frontier", TITLE); \
            goto exit;
    }

    /*****************
     * Check Balance *
     *****************/
    if (mbedtls_mpi_cmp_mpi(&(frontier_block.balance), &transaction_amount) == -1) {
        loading_disable();
        ESP_LOGI(TAG, "Insufficent Funds.");
        menu8g2_display_text_title(&menu, "Insufficent Funds", TITLE);
        goto exit;
    }
    
    /*********************
     * Create send block *
     *********************/
    loading_text_title("Creating Block", TITLE);
    
    sodium_memzero(&rpc, sizeof(rpc));
    rpc.type = NANO_BLOCK_SIGN;
    rpc.nano_block_sign.index = index;
    rpc.nano_block_sign.frontier = frontier_block;
    nl_block_t *new_block = &(rpc.nano_block_sign.block);

    new_block->type = STATE;
    sodium_hex2bin(new_block->previous, sizeof(new_block->previous),
            frontier_hash, sizeof(frontier_hash), NULL, NULL, NULL);
    memcpy(new_block->account, my_public_key, sizeof(my_public_key));
    memcpy(new_block->representative, frontier_block.representative, BIN_256);
    memcpy(new_block->link, dest_public_key, sizeof(dest_public_key));
    mbedtls_mpi_sub_abs(&(new_block->balance), &(frontier_block.balance), &transaction_amount);

    #if LOG_LOCAL_LEVEL >= ESP_LOG_INFO
    {
    char amount[66];
    size_t olen;
    mbedtls_mpi_write_string(&(frontier_block.balance), 10, amount, sizeof(amount), &olen);
    ESP_LOGI(TAG, "Frontier Amount: %s", amount);
    mbedtls_mpi_write_string(&(new_block->balance), 10, amount, sizeof(amount), &olen);
    ESP_LOGI(TAG, "New Block Amount: %s", amount);
    mbedtls_mpi_write_string(&transaction_amount, 10, amount, sizeof(amount), &olen);
    ESP_LOGI(TAG, "Transaction Amount: %s", amount);
    }
    #endif

    // Prompt and Sign block
    loading_disable();
    if(vault_rpc(&rpc) != RPC_SUCCESS){
        return;
    }
    loading_enable();

    // Get RECEIVE work
    loading_text_title("Fetching Work", TITLE);
    if( E_SUCCESS != nanoparse_lws_work( frontier_hash, &proof_of_work ) ){
        ESP_LOGI(TAG, "Invalid Work (RECEIVE) Response.");
        loading_disable();
        menu8g2_display_text_title(&menu, "Failed Fetching Work", TITLE); \
        goto exit;
    }
    new_block->work = proof_of_work;

    loading_text_title("Broadcasting", TITLE);
    switch(nanoparse_lws_process(new_block)){
        case E_SUCCESS:
            break;
        default:
            loading_disable();
            menu8g2_display_text_title(&menu, "Error Broadcasting", TITLE);
            goto exit;
    }

    
    loading_disable();
    menu8g2_display_text_title(&menu, "Transaction Sent", TITLE);

    exit:
        loading_disable();
        return;
}
