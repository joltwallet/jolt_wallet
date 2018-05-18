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

#include "nano_lws.h"
#include "nano_parse.h"

static const char TAG[] = "nano_send";

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
    loading_text("Enter Send Details");
    
    char dest_address[ADDRESS_BUF_LEN];
    flush_uart();
    printf("\nEnter a destination address: ");
    get_serial_input(dest_address, sizeof(dest_address));

	uint8_t size = strlen(dest_address);
    // Check prefix and exclude it from the buffer
    if ((dest_address[0] == 'n' || dest_address[0] == 'N') &&
        (dest_address[1] == 'a' || dest_address[1] == 'A') &&
        (dest_address[2] == 'n' || dest_address[2] == 'N') &&
        (dest_address[3] == 'o' || dest_address[3] == 'O') &&
        (dest_address[4] == '-' || dest_address[4] == '_')) {
        if (size != ADDRESS_DATA_LEN + 5) {
            return E_INVALID_ADDRESS;
        }
    } else if ((dest_address[0] == 'x' || dest_address[0] == 'X') &&
               (dest_address[1] == 'r' || dest_address[1] == 'R') &&
               (dest_address[2] == 'b' || dest_address[2] == 'B') &&
               (dest_address[3] == '-' || dest_address[3] == '_')) {
        if (size != ADDRESS_DATA_LEN + 4) {
            return E_INVALID_ADDRESS;
        }
    } else if (size == ADDRESS_DATA_LEN){
        // continue; assumes address doesn't have a prefix
    } else {
        loading_disable();
        menu8g2_display_text(&menu, "Incorrect Address");
        ESP_LOGE(TAG, "\nIncorrect Address %s\n", dest_address);
        return E_INVALID_ADDRESS;
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
    loading_disable();
    nvs_handle nvs_secret;
    
    init_nvm_namespace(&nvs_secret, "secret");
    if(ESP_OK != nvs_get_u32(nvs_secret, "index", &(rpc.nano_public_key.index))){
        rpc.nano_public_key.index = 0;
    }
    nvs_close(nvs_secret);

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
    loading_enable();
    loading_text("Getting Block Details");
    
    hex256_t frontier_hash = { 0 };
    nl_block_t frontier_block;
    nl_block_init(&frontier_block);
    uint64_t proof_of_work;

    if( get_frontier(my_address, frontier_hash) == E_SUCCESS ){
        ESP_LOGI(TAG, "Creating SEND Block");

        if( get_block(frontier_hash, &frontier_block) != E_SUCCESS ){
            loading_disable();
            ESP_LOGI(TAG, "Error retrieving frontier block.");
            return;
        }

        // Get SEND work
        if( E_SUCCESS != get_work( frontier_hash, &proof_of_work ) ){
            loading_disable();
            ESP_LOGI(TAG, "Invalid Work (SEND) Response.");
            return;
        }

    }
    else {
        //To send requires a previous Open Block
        loading_disable();
        ESP_LOGI(TAG, "Account not open.");
        return;
    }

    /*****************
     * Check Balance *
     *****************/
    if (mbedtls_mpi_cmp_mpi(&(frontier_block.balance), &transaction_amount) == -1) {
        loading_disable();
        ESP_LOGI(TAG, "Insufficent Funds.");
        menu8g2_display_text(&menu, "Insufficent Funds");
        return;
    }
    
    /*********************
     * Create send block *
     *********************/
    
    loading_text("Creating Send Block");
    
    sodium_memzero(&rpc, sizeof(rpc));
    rpc.type = NANO_BLOCK_SIGN;
    nl_block_t *new_block = &(rpc.nano_block_sign.block);

    new_block->type = STATE;
    sodium_hex2bin(new_block->previous, sizeof(new_block->previous),
            frontier_hash, sizeof(frontier_hash), NULL, NULL, NULL);
    memcpy(new_block->account, my_public_key, sizeof(my_public_key));
    nl_address_to_public(new_block->representative, my_address); //todo: default rep
    nl_address_to_public(new_block->link, dest_address);
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
    if(vault_rpc(&rpc) != RPC_SUCCESS){
        return;
    }
    
    loading_text("Broadcasting Transaction");
    process_block(new_block);
    
    loading_disable();
    menu8g2_display_text(&menu, "Transaction Sent");
}
