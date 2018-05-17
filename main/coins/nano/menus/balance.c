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

#include "nano_lws.h"
#include "nano_parse.h"

static const char TAG[] = "nano_balance";

void menu_nano_balance(menu8g2_t *prev){
    /*
     * Blocks involved:
     * prev_block - frontier of our account chain
     */
    
    vault_rpc_t rpc;
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);
    
    /******************
     * Get My Address *
     ******************/
    nvs_handle nvs_secret;
    
    init_nvm_namespace(&nvs_secret, "secret");
    nvs_get_u32(nvs_secret, "index", &(rpc.public_key.index));
    nvs_close(nvs_secret);

    sodium_memzero(&rpc, sizeof(rpc));
    rpc.type = NANO_PUBLIC_KEY;
    if(vault_rpc(&rpc) != RPC_SUCCESS){
        return;
    }
    uint256_t my_public_key;
    memcpy(my_public_key, rpc.public_key.block.account, sizeof(my_public_key));

    char my_address[ADDRESS_BUF_LEN];
    nl_public_to_address(my_address, sizeof(my_address), my_public_key);
    
    ESP_LOGI(TAG, "My Address: %s\n", my_address);

    /********************************************
     * Get My Account's Frontier Block *
     ********************************************/
    // Assumes State Blocks Only
    // Outcome:
    //     * frontier_hash, frontier_block
    hex256_t frontier_hash = { 0 };
    nl_block_t frontier_block;
    nl_block_init(&frontier_block);
    uint64_t proof_of_work;

    if( get_frontier(my_address, frontier_hash) == E_SUCCESS ){
        
        if( get_block(frontier_hash, &frontier_block) != E_SUCCESS ){
            ESP_LOGI(TAG, "Error retrieving frontier block.");
            return;
        }

    }
    else {
        //To send requires a previous Open Block
        ESP_LOGI(TAG, "Account not open.");
        return;
    }

    /*****************
     * Check Balance *
     *****************/
    
    char amount[64];
    size_t olen;
    mbedtls_mpi_write_string(&(frontier_block.balance), 10, amount, sizeof(amount), &olen);
    
    ESP_LOGI(TAG, "Frontier Amount: %s", amount);
    
    char balance_string[75];
    snprintf(balance_string, 64, "%s Raw Nano", amount);

    bool statusbar_draw_original = statusbar_draw_enable;
    
    statusbar_draw_enable = false;
    menu.post_draw = NULL;
    for(;;){
        if(menu8g2_display_text(&menu, balance_string) == (1ULL << EASY_INPUT_BACK)){
            statusbar_draw_enable = statusbar_draw_original;
            return;
        }
    }
}
