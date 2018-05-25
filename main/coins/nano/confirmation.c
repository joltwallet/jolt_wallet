#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_log.h"
#include "sodium.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "menu8g2.h"
#include "nano_lib.h"

#include "../../vault.h"
#include "../../globals.h"

static const char *TAG = "nano_conf";


static bool draw_screen(menu8g2_t *menu, char *buf){
    /* Draws screen for yes/no action */
    uint64_t button;
    for(;;){
        button = menu8g2_display_text_title(menu, buf, "Confirm Action");
        if((1ULL << EASY_INPUT_BACK) & button){
            return false;
        }
        else if((1ULL << EASY_INPUT_ENTER) & button){
            return true;
        }
    }
}

bool nano_confirm_contact_update(const menu8g2_t *prev_menu, const char *name, const uint256_t public, const uint8_t index){
    menu8g2_t menu;
    menu8g2_copy(&menu, prev_menu);

    char buf[200];
    snprintf(buf, sizeof(buf), "Update Index: %d ?", index);
    if ( !draw_screen(&menu, buf) ){
        return false;
    }

    snprintf(buf, sizeof(buf), "Name: %s", name);
    if ( !draw_screen(&menu, buf) ){
        return false;
    }

    char address[ADDRESS_BUF_LEN];
    if( E_SUCCESS != nl_public_to_address(address, sizeof(address), public) ){
        return false;
    }
    snprintf(buf, sizeof(buf), "Address: %s", address);
    if ( !draw_screen(&menu, buf) ){
        return false;
    }

    return true;
}

bool nano_confirm_block(menu8g2_t *prev_menu, nl_block_t *head_block, nl_block_t *new_block){
    /* Prompts user to confirm transaction information before signing
     * Expects State Blocks 
     * Returns true on affirmation, false on error or cancellation.
     * */
    menu8g2_t menu;
    menu8g2_copy(&menu, prev_menu);

    bool result = false;
    double display_amount;

    mbedtls_mpi transaction_amount;
    mbedtls_mpi_init(&transaction_amount);

    if(head_block->type == STATE){
        // Make sure the new_block's prev is the head_block
        uint256_t head_block_hash;
        nl_block_compute_hash(head_block, head_block_hash);
        if(0 != memcmp(head_block_hash, new_block->previous, BIN_256)){
            goto exit;
        }

        // Reject Invalid negative balances
        if(-1 == new_block->balance.s || -1 == head_block->balance.s){
            goto exit;
        }

        /******************************
         * Compute transaction amount *
         ******************************/
        mbedtls_mpi_sub_mpi(&transaction_amount, &(head_block->balance), &(new_block->balance));
        if( E_SUCCESS != nl_mpi_to_nano_double(&transaction_amount, &display_amount) ){
            goto exit;
        }

        char address[ADDRESS_BUF_LEN];
        char buf[200];
        if(0 != memcmp(head_block->representative, new_block->representative, BIN_256)){
            ESP_LOGI(TAG, "Detected Representative Change");
            /* Translate New Rep Address */
            if(E_SUCCESS != nl_public_to_address(address, sizeof(address),
                    new_block->representative)){
                goto exit;
            }
            snprintf(buf, sizeof(buf), "Change Rep to %s ?", address);
            if( draw_screen(&menu, buf) ){
                result = true;
            }
            else{
                goto exit;
            }
        }
        if( display_amount > 0){
            ESP_LOGI(TAG, "Detected Send");
            /* Translate Destination Address */
            if(E_SUCCESS != nl_public_to_address(address, sizeof(address),
                    new_block->link)){
                goto exit;
            }

            snprintf(buf, sizeof(buf), "Send %.3lf NANO to %s ?", display_amount, address);
            if( draw_screen(&menu, buf) ){
                result = true;
            }
            else{
                goto exit;
            }
        }
        if( display_amount < 0){
            ESP_LOGI(TAG, "Detected Receive");
            // Auto Receive
            result = true;
        }
    }
    else if(head_block->type == UNDEFINED){
        ESP_LOGI(TAG, "No Frontier, verifying prevhash == 0");
        // new block must be an open state
        for(int i = 0; i < 32; i++){
            if( 0 != (new_block->previous)[i] ){
                goto exit;
            }
        }
        result = true;
    }
    else{
        ESP_LOGI(TAG, "Cannot verify with Legacy Block");
    }

    exit:
        mbedtls_mpi_free(&transaction_amount);
        return result;
}
