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
#include "../../console.h"
#include "../../gui.h"


static const char* TAG = "console_nano";

static uint32_t get_nvs_index(){
    uint32_t index;
    nvs_handle nvs_h;
    init_nvm_namespace(&nvs_h, "nano");
    if(ESP_OK != nvs_get_u32(nvs_h, "index", &index)){
        index = 0;
    }
    nvs_close(nvs_h);
    return index;

}

static int nano_sign_block(int argc, char** argv) {
    /* Given the index, head block, and the block to be signed,
     * prompt user to sign */
    if( !console_check_equal_argc(argc, 4) ){
        return 1;
    }

    SCREEN_SAVE;

    int response = -1;

    nl_block_t head, new;
    nl_block_init(&head);
    nl_block_init(&new);

    /* Parse Blocks Into Json */
    if( E_SUCCESS != nanoparse_block(argv[2], &head) ){
        printf("Unable to parse head block\n");
        response = 2;
        goto exit;
    }
    if( E_SUCCESS != nanoparse_block(argv[3], &new) ){
        printf("Unable to parse block to be signed\n");
        response = 3;
        goto exit;
    }
    uint32_t index;
    index = atoi( argv[1] );
    if( 0 == index && '0' != argv[1][0]) {
        printf("Invalid Index");
        response = 4;
        goto exit;
    }

    /* Sign Block */
    vault_rpc_t rpc= {
        .type = NANO_BLOCK_SIGN,
        .nano_block_sign.index = index,
    };
    nl_block_copy(&(rpc.nano_block_sign.frontier), &head);
    nl_block_copy(&(rpc.nano_block_sign.block), &new);

    if(vault_rpc(&rpc) != RPC_SUCCESS){
        printf("Unable to sign block\n");
        response = 5;
        goto exit;
    }

    /* Parse Signed Block Back Into Json */
    char buf[1024];
    if( E_SUCCESS != nanoparse_process(
                &(rpc.nano_block_sign.block), buf, sizeof(buf)) ){
        printf("Error Parsing Signed Block\n");
        response = 6;
        goto exit;
    }

    printf("Signed Block:\n%s\n", buf);
    response = 0;

    exit:
        SCREEN_RESTORE;
        nl_block_free(&head);
        nl_block_free(&new);
        return response;
}

static int nano_count(int argc, char** argv) {
    /* Gets Nano Server's Block Count */
    uint32_t block_count = nanoparse_lws_block_count();
    printf("%d Blocks\n", block_count);
    return 0;
}

static int nano_address(int argc, char ** argv){
    /* Return The Addresses of given index
     * Optionally takes a second argument to return the range */

    int response;
    uint32_t lower, upper;
    if( !console_check_range_argc(argc, 1, 3) ){
        return 1;
    }

    SCREEN_SAVE;

    if( 1 == argc ){
        lower = get_nvs_index();
        upper = lower;
    }
    else{
        lower = atoi( argv[1] );
        if( 3 == argc ){
            upper = atoi(argv[2]);
        }
        else {
            upper = lower;
        }
    }

    for(uint32_t index=lower; index<=upper; index++ ){
        vault_rpc_t rpc;
        rpc.nano_public_key.index = index;
        rpc.type = NANO_PUBLIC_KEY;
        if(vault_rpc(&rpc) != RPC_SUCCESS){
            printf("User cancelled.\n");
            response = 2;
            goto exit;
        }
        char address[ADDRESS_BUF_LEN];
        nl_public_to_address(address, sizeof(address),
                rpc.nano_public_key.block.account);
        printf("Index: %d; Address: %s\n", index, address);
    }
    response = 0;
    exit:
        SCREEN_RESTORE;
        return response;
}

static int nano_balance(int argc, char** argv) {
    /* Gets currently selected Nano account's balance.
     * Takes optional 1 argument: integer index */
    int response = -1;

    if( !console_check_range_argc(argc, 1, 2) ){
        return 1;
    }

    SCREEN_SAVE;

    double display_amount;
    
    /******************
     * Get My Address *
     ******************/
    uint32_t index;
    if( argc == 2 ){
        index = atoi( argv[1] );
        if( 0 == index && '0' != argv[1][0]) {
            printf("Invalid Index");
            response = 2;
            goto exit;
        }
    }
    else{
        index = get_nvs_index();
    }

    vault_rpc_t rpc;
    rpc.nano_public_key.index = index;
    rpc.type = NANO_PUBLIC_KEY;
    if(vault_rpc(&rpc) != RPC_SUCCESS){
        printf("User cancelled.\n");
        response = 3;
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
                display_amount = 4;
                goto exit;
            }
            break;
        default:
            display_amount = 0;
            break;
    }

    if( display_amount >= 0 ){
        printf("Balance: %0.4lf Nano\n", display_amount);
        response = 0;
    }
    else{
        printf("Error\n");
        response = 5;
    }

    exit:
        SCREEN_RESTORE;
        return response;
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

    cmd = (esp_console_cmd_t) {
        .command = "nano_address",
        .help = "Get the Nano Address at derivation index or index range",
        .hint = NULL,
        .func = &nano_address,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "nano_sign_block",
        .help = "Given the index, head block, and the block to be signed,"
                "prompt user to sign",
        .hint = NULL,
        .func = &nano_sign_block,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
