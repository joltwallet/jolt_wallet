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

#define NANO_DISP_DECIMAL 2

bool nano_confirm_block(menu8g2_t *prev_menu, nl_block_t *head_block, nl_block_t *new_block){
    /* Prompts user to confirm transaction information before signing
     * Expects State Blocks 
     * Returns true on affirmation, false on error or cancellation.
     * */
    menu8g2_t menu;
    menu8g2_copy(&menu, prev_menu);
    menu.post_draw = NULL;
    statusbar_draw_enable = false;

    bool statusbar_draw_original = statusbar_draw_enable;
    bool result;
    double display_amount;
    char address[ADDRESS_BUF_LEN];


    // Only accept state head blocks 
    if(head_block->type != STATE){
        result = false;
        goto exit;
    }

    // Reject Invalid negative balances
    if(-1 == new_block->balance.s || -1 == head_block->balance.s){
        result = false;
        goto exit;
    }

    /******************************
     * Compute transaction amount *
     ******************************/
    mbedtls_mpi transaction_amount;
    mbedtls_mpi_init(&transaction_amount);
    mbedtls_mpi_sub_mpi(&transaction_amount, &(head_block->balance), &(new_block->balance));
    if( E_SUCCESS != nl_mpi_to_nano_double(&transaction_amount, &display_amount) ){
        result = false;
        goto exit;
    }


    char buf[100];
    if(0 == mbedtls_mpi_cmp_mpi(&(head_block->balance), &(new_block->balance))){
        // Change Block
        // todo: implement
        buf[0] = '\0';
    }
    else if( display_amount > 0){
        // Send
        /* Translate Destination Address */
        if(E_SUCCESS != nl_public_to_address(address, sizeof(address),
                new_block->link)){
            result = false;
            goto exit;
        }

        /* Write out display string */
        snprintf(buf, sizeof(buf), "Send %.2lf NANO to %s ?", display_amount, address);
    }
    else{
        // Receive
        // todo: implement
        buf[0] = '\0';
    }

    /***************
     * Draw Screen *
     ***************/
    for(;;){
        switch(menu8g2_display_text(&menu, buf)){
            case(1ULL << EASY_INPUT_BACK):
                result = false;
                goto exit;
            case(1ULL << EASY_INPUT_ENTER):
                result = true;
                goto exit;
            default:
                break;
        }
    }

    exit:
        statusbar_draw_enable = statusbar_draw_original;
        mbedtls_mpi_free(&transaction_amount);
        return result;
}
