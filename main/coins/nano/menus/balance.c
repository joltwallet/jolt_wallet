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
#include "../../../gui.h"
#include "../../../loading.h"

#include "nano_lws.h"
#include "nano_parse.h"

static const char TAG[] = "nano_balance";
static const char TITLE[] = "Nano Balance";

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
    nvs_handle nvs_h;
    init_nvm_namespace(&nvs_h, "nano");
    if(ESP_OK != nvs_get_u32(nvs_h, "index", &(rpc.nano_public_key.index))){
        rpc.nano_public_key.index = 0;
    }
    nvs_close(nvs_h);

    sodium_memzero(&rpc, sizeof(rpc));
    rpc.type = NANO_PUBLIC_KEY;
    if(vault_rpc(&rpc) != RPC_SUCCESS){
        return;
    }
    uint256_t my_public_key;
    memcpy(my_public_key, rpc.nano_public_key.block.account, sizeof(my_public_key));

    char my_address[ADDRESS_BUF_LEN];
    nl_public_to_address(my_address, sizeof(my_address), my_public_key);
    
    ESP_LOGI(TAG, "My Address: %s\n", my_address);

    /********************************************
     * Get My Account's Frontier Block *
     ********************************************/
    // Assumes State Blocks Only
    // Outcome:
    //     * frontier_hash, frontier_block
    loading_enable();
    loading_text_title("Getting Frontier", TITLE);

    hex256_t frontier_hash = { 0 };
    nl_block_t frontier_block;
    nl_block_init(&frontier_block);

    if( get_frontier(my_address, frontier_hash) == E_SUCCESS ){
        
        if( get_block(frontier_hash, &frontier_block) != E_SUCCESS ){
            ESP_LOGI(TAG, "Error retrieving frontier block.");
            goto exit;
        }

    }
    else {
        //To send requires a previous Open Block
        ESP_LOGI(TAG, "Account not open.");
        goto exit;
    }

    /*****************
     * Check Balance *
     *****************/
    loading_disable();
    double display_amount;
    if( E_SUCCESS != nl_mpi_to_nano_double(&(frontier_block.balance),
                &display_amount) ){
        // Error
        goto exit;
    }

    ESP_LOGI(TAG, "Approximate Account Balance: %0.3lf", display_amount);
    
    char buf[100];
    snprintf(buf, sizeof(buf), "%0.3lf Nano", display_amount);

    for(;;){
        if(menu8g2_display_text_title(&menu, buf, TITLE)
                == (1ULL << EASY_INPUT_BACK)){
            goto exit;
        }
    }

    exit:
        return;

}
