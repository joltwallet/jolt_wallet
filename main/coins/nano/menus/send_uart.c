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

#include "nano_lws.h"
#include "nano_parse.h"

static const char TAG[] = "nano_send";
static const char TITLE[] = "Send Nano";

static void get_serial_input(char *serial_rx, int buffersize){
    
    int line_pos = 0;
    
    while(1){
        int c = getchar();
        
        if(c < 0) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            continue;
        }
        if(c == '\r') continue;
        if(c == '\n') {
            
            // terminate the string
            serial_rx[line_pos] = '\0';
            printf("\n");
            break;
        }
        else {
            putchar(c);
            serial_rx[line_pos] = c;
            line_pos++;
            
            // buffer full!
            if(line_pos == buffersize) {
                
                printf("\nCommand buffer full!\n");
                serial_rx[line_pos] = '\0';
                
                break;
            }
        }
        
        
    }
}

static void get_serial_input_int(char *serial_rx, const int buffersize){
    // fills up serial_rx with an ascii string where all characters must be ints
    
    int line_pos = 0;
    
    while(1){
        int c = getchar();
        
        if(c < 0) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            continue;
        }
        if(c == '\r') continue;
        if(c == '\n') {
            // terminate the string
            serial_rx[line_pos] = '\0';
            printf("\n");
            break;
        }
        else {
            if( c >= '0' && c <= '9' ){
                putchar(c);
                serial_rx[line_pos] = c;
                line_pos++;
                
                // buffer full!
                if(line_pos == buffersize) {
                    printf("\nCommand buffer full!\n");
                    serial_rx[line_pos] = '\0';
                    break;
                }
            }
        }
        
        
    }
}

static void flush_uart(){
    //This is a terrible hack to flush the uarts buffer, a far better option would be rewrite all uart related code
    // to use proper uart code from driver/uart.h
    for(int bad_hack = 0; bad_hack <= 10; bad_hack++){
        getchar();
    };
}

void menu_nano_send_uart(menu8g2_t *prev){
    /*
     * Blocks involved:
     * new_block - block we are creating
     * prev_block - frontier of our account chain
     */
    
    vault_rpc_t rpc;
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);
    menu.post_draw = NULL;

    /**************************************
     * Get Destination Address and Amount *
     **************************************/
    loading_enable();
    loading_text_title("Enter Send Details", TITLE);
    
    char dest_address[ADDRESS_BUF_LEN];
    flush_uart();
    printf("\nEnter a destination address: ");
    get_serial_input(dest_address, sizeof(dest_address));

    // Verify Destination Address
    uint256_t dest_public_key;
    if(E_SUCCESS != nl_address_to_public(dest_public_key, dest_address)){
        loading_disable();
        menu8g2_display_text_title(&menu, "Invalid Address", TITLE); \
        ESP_LOGE(TAG, "\nInvalid Address %s\n", dest_address); \
        goto exit;
    }

    flush_uart();
    printf("destination address: %s\n", dest_address);
    
    char dest_amount_buf[40];
    printf("\nEnter amount (raw): ");
    get_serial_input_int(dest_amount_buf, sizeof(dest_amount_buf)); //This only accepts numbers
    
    mbedtls_mpi transaction_amount;
    mbedtls_mpi_init(&transaction_amount);
    mbedtls_mpi_read_string(&transaction_amount, 10, dest_amount_buf);
    
    /******************
     * Get My Address *
     ******************/
    //loading_disable();
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
     * Get My Account's Frontier Block and Work *
     ********************************************/
    // Assumes State Blocks Only
    // Outcome:
    //     * frontier_hash, frontier_block
    //loading_enable();
    loading_text_title("Getting Block Details", TITLE);
    
    hex256_t frontier_hash = { 0 };
    nl_block_t frontier_block;
    nl_block_init(&frontier_block);
    uint64_t proof_of_work;

    if( get_frontier(my_address, frontier_hash) == E_SUCCESS ){
        ESP_LOGI(TAG, "Creating SEND Block");

        if( get_block(frontier_hash, &frontier_block) != E_SUCCESS ){
            ESP_LOGI(TAG, "Error retrieving frontier block.");
            goto exit;
        }

        // Get SEND work
        loading_text_title("Fetching Work", TITLE);
        if( E_SUCCESS != get_work( frontier_hash, &proof_of_work ) ){
            ESP_LOGI(TAG, "Invalid Work (SEND) Response.");
            loading_disable();
            menu8g2_display_text_title(&menu, "Failed Fetching Work", TITLE); \
            goto exit;
        }
    }
    else {
        //To send requires a previous Open Block
        ESP_LOGI(TAG, "Account not open.");
        loading_disable();
        menu8g2_display_text_title(&menu, "Account Not Open", TITLE); \
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
    loading_text_title("Creating Send Block", TITLE);
    
    sodium_memzero(&rpc, sizeof(rpc));
    rpc.type = NANO_BLOCK_SIGN;
    nl_block_t *new_block = &(rpc.nano_block_sign.block);

    new_block->type = STATE;
    sodium_hex2bin(new_block->previous, sizeof(new_block->previous),
            frontier_hash, sizeof(frontier_hash), NULL, NULL, NULL);
    memcpy(new_block->account, my_public_key, sizeof(my_public_key));
    nl_address_to_public(new_block->representative, my_address); //todo: default rep
    memcpy(new_block->link, dest_public_key, sizeof(dest_public_key));
    mbedtls_mpi_sub_abs(&(new_block->balance), &(frontier_block.balance), &transaction_amount);
    new_block->work = proof_of_work;

    #if LOG_LOCAL_LEVEL >= ESP_LOG_INFO
    {
    char amount[64];
    size_t olen;
    mbedtls_mpi_write_string(&(frontier_block.balance), 10, amount, sizeof(amount), &olen);
    ESP_LOGI(TAG, "Frontier Amount: %s", amount);
    mbedtls_mpi_write_string(&(new_block->balance), 10, amount, sizeof(amount), &olen);
    ESP_LOGI(TAG, "New Block Amount: %s", amount);
    mbedtls_mpi_write_string(&transaction_amount, 10, amount, sizeof(amount), &olen);
    ESP_LOGI(TAG, "Transaction Amount: %s", amount);
    }
    #endif

    // Sign block
    loading_disable();
    if( nano_confirm_block(&menu, &frontier_block, new_block) ){
        if(vault_rpc(&rpc) != RPC_SUCCESS){
            return;
        }
    }
    else{
        return;
    }

    loading_enable();
    loading_text_title("Broadcasting Transaction", TITLE);
    process_block(new_block);
    
    loading_disable();
    menu8g2_display_text_title(&menu, "Transaction Sent", TITLE);

    exit:
        loading_disable();
        return;
}
