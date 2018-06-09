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
#include "../../statusbar.h"
#include "../../loading.h"


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

static int nano_send(int argc, char** argv) {
    /*
     * Account Index, Dest Address, Amount (raw)
     */

    if( !console_check_equal_argc(argc, 4) ){
        return -1;
    }
    SCREEN_SAVE;

    vault_rpc_t rpc;
    menu8g2_t menu;
    menu8g2_init(&menu, (u8g2_t *) &u8g2, input_queue, disp_mutex, NULL, statusbar_update);
    int return_code = 0;
    const char *TITLE = "Nano Send";

    /**************************************
     * Get Destination Address and Amount *
     **************************************/
    loading_enable();
    
    // Verify Destination Address
    uint256_t dest_public_key;
    if(E_SUCCESS != nl_address_to_public(dest_public_key, argv[2])){
        loading_disable();
        menu8g2_display_text_title(&menu, "Invalid Address", TITLE);
        ESP_LOGE(TAG, "\nInvalid Address %s\n", argv[2]);
        return_code = 1;
        goto exit;
    }

    mbedtls_mpi transaction_amount;
    mbedtls_mpi_init(&transaction_amount);
    if( 0 != mbedtls_mpi_read_string(&transaction_amount, 10, argv[3]) ) {
        loading_disable();
        menu8g2_display_text_title(&menu, "Invalid Amount", TITLE);
        ESP_LOGE(TAG, "\nInvalid Amount %s\n", argv[2]);
        return_code = 2;
        goto exit;
    }
    
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
        return_code = 3;
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
            menu8g2_display_text_title(&menu, "Could not fetch frontier", TITLE);
            return_code = 4;
            goto exit;
    }

    /*****************
     * Check Balance *
     *****************/
    if (mbedtls_mpi_cmp_mpi(&(frontier_block.balance), &transaction_amount) == -1) {
        loading_disable();
        ESP_LOGI(TAG, "Insufficent Funds.");
        menu8g2_display_text_title(&menu, "Insufficent Funds", TITLE);
        return_code = 5;
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
        return_code = 6;
        goto exit;
    }
    loading_enable();

    // Get RECEIVE work
    loading_text_title("Fetching Work", TITLE);
    if( E_SUCCESS != nanoparse_lws_work( frontier_hash, &proof_of_work ) ){
        ESP_LOGI(TAG, "Invalid Work (RECEIVE) Response.");
        loading_disable();
        menu8g2_display_text_title(&menu, "Failed Fetching Work", TITLE);
        return_code = 7;
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
            return_code = 8;
            goto exit;
    }

    
    loading_disable();
    menu8g2_display_text_title(&menu, "Transaction Sent", TITLE);

    exit:
        loading_disable();
        SCREEN_RESTORE;
        return return_code;
}

static int nano_contact_update(int argc, char** argv) {
    /* Updates a nano contact.
     * (contact index, contact name, contact address)
     */

    int return_code = 0;
    if( !console_check_equal_argc(argc, 4) ){
        return_code = -1;
        goto exit;
    }

    uint8_t contact_index = atoi(argv[1]);
    if( contact_index >= CONFIG_JOLT_NANO_CONTACTS_MAX ){
        printf("Contact index must be smaller than %d.\n",
                CONFIG_JOLT_NANO_CONTACTS_MAX);
        return_code = 1;
        goto exit;
    }

    if( strlen(argv[2]) >= CONFIG_JOLT_NANO_CONTACTS_NAME_LEN ) {
        printf("Shortening Name.\n");
        argv[2][CONFIG_JOLT_NANO_CONTACTS_NAME_LEN] = '\0';
    }

    uint256_t contact_public_key;
    if( E_SUCCESS != nl_address_to_public(contact_public_key, argv[3]) ){
        printf("Invalid Address.\n");
        return_code = 2;
        goto exit;
    }

    vault_rpc_t rpc = {
        .type = NANO_CONTACT_UPDATE,
        .nano_contact_update.index = contact_index,
        .nano_contact_update.name = argv[2],
        .nano_contact_update.public = contact_public_key
    };

    if(vault_rpc(&rpc) != RPC_SUCCESS){
        return_code = 3;
        goto exit;
    }

    exit:
        return return_code;
}

static int nano_sign_block(int argc, char** argv) {
    /* Given the index, head block, and the block to be signed,
     * prompt user to sign */
    if( !console_check_equal_argc(argc, 4) ){
        return 1;
    }

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
        return response;
}

static int nano_balance(int argc, char** argv) {
    /* Gets currently selected Nano account's balance.
     * Takes optional 1 argument: integer index */
    int response = -1;

    if( !console_check_range_argc(argc, 1, 2) ){
        return 1;
    }

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
            printf("Unable to get account's frontier.\n");
            response = 6;
            goto exit;
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

    cmd = (esp_console_cmd_t) {
        .command = "nano_send",
        .help = "WiFi send. Inputs: account index, dest address, amount (raw)",
        .hint = NULL,
        .func = &nano_send,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "nano_contact_update",
        .help = "Update Nano Contact (index, name, address)",
        .hint = NULL,
        .func = &nano_contact_update,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}
