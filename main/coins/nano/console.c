#include "menu8g2.h"
#include "sodium.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "linenoise/linenoise.h"
#include "esp_log.h"
#include "esp_console.h"
#include <libwebsockets.h>

#include "nano_lws.h"
#include "nano_parse.h"
#include "../../globals.h"
#include "../../vault.h"
#include "../../loading.h"


static const char* TAG = "console_nano";

static int nano_count(int argc, char** argv) {
    uint32_t block_count = nanoparse_lws_block_count();
    printf("%d Blocks\n", block_count);
    return 0;
}

static int nano_balance(int argc, char** argv) {
    size_t disp_buffer_size = 8 * u8g2_GetBufferTileHeight(&u8g2) *
            u8g2_GetBufferTileWidth(&u8g2);
    uint8_t *old_disp_buffer = malloc(disp_buffer_size);
    memcpy(old_disp_buffer, u8g2_GetBufferPtr(&u8g2), disp_buffer_size);

    vault_rpc_t rpc;
    double display_amount;
    
    /******************
     * Get My Address *
     ******************/
    nvs_handle nvs_h;
    init_nvm_namespace(&nvs_h, "nano");
    if(ESP_OK != nvs_get_u32(nvs_h, "index", &(rpc.nano_public_key.index))){
        rpc.nano_public_key.index = 0;
    }
    nvs_close(nvs_h);
    
    rpc.type = NANO_PUBLIC_KEY;
    if(vault_rpc(&rpc) != RPC_SUCCESS){
        printf("User cancelled.\n");
        display_amount = -1;
        goto exit;
    }
    char address[ADDRESS_BUF_LEN];
    nl_public_to_address(address, sizeof(address),
            rpc.nano_public_key.block.account);
    printf("Address: %s\n", address);
    
    /********************************************
     * Get My Account's Frontier Block *
     ********************************************/
    // Assumes State Blocks Only
    // Outcome:
    //     * frontier_hash, frontier_block
    nl_block_t frontier_block;
    nl_block_init(&frontier_block);
    memcpy(frontier_block.account, rpc.nano_public_key.block.account, BIN_256);
    
    switch( nanoparse_lws_frontier_block(&frontier_block) ){
        case E_SUCCESS:
            if( E_SUCCESS != nl_mpi_to_nano_double(&(frontier_block.balance),
                                                   &display_amount) ){
                display_amount = -1;
            }
            break;
        default:
            display_amount = 0;
            break;
    }

    if( display_amount >= 0 ){
        printf("Balance: %0.4lf Nano\n", display_amount);
    }
    else{
        printf("Error\n");
    }

    exit:
        memcpy(u8g2_GetBufferPtr(&u8g2), old_disp_buffer, disp_buffer_size);
        free(old_disp_buffer);
        return display_amount;
}

void console_nano_register() {
    esp_console_cmd_t cmd;

    cmd = (esp_console_cmd_t) {
        .command = "nano_count",
        .help = "Get the current Nano block count",
        .hint = NULL,
        .func = &nano_count,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "nano_balance",
        .help = "Get the current Nano Balance",
        .hint = NULL,
        .func = &nano_balance,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
