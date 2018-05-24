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

#include "nvs_flash.h"
#include "nvs.h"
#include "nano_lib.h"
#include "secure_entry.h"
#include "vault.h"
#include "globals.h"
#include "statusbar.h"
#include "loading.h"
#include "helpers.h"

// Coin RPCs
#include "rpc_syscore.h"
#include "coins/nano/rpc.h"

static const char* TAG = "vault";
static const char* TITLE = "Vault Access";

vault_rpc_response_t vault_rpc(vault_rpc_t *rpc){
    /* Sets up rpc queue, blocks until vault responds. */
    vault_rpc_response_t res;

    #if LOG_LOCAL_LEVEL >= ESP_LOG_INFO
    int32_t id = randombytes_random();
    #endif

    rpc->response_queue = xQueueCreate( 1, sizeof(res) );
    
    ESP_LOGI(TAG, "Attempting Vault RPC Send %d; ID: %d", rpc->type, id);
    if(xQueueSend( vault_queue, (void *) &rpc, 0)){
        ESP_LOGI(TAG, "Success: Vault RPC Send %d; ID: %d", rpc->type, id);
        ESP_LOGI(TAG, "Awaiting Vault RPC Send %d response; ID: %d", rpc->type, id);
        xQueueReceive(rpc->response_queue, (void *) &res, portMAX_DELAY);
        ESP_LOGI(TAG, "Success: Vault RPC Send %d response %d; ID: %d",
                rpc->type, res, id);
    }
    else{
        ESP_LOGI(TAG, "Vault RPC Send %d failed; ID: %d", rpc->type, id);
        res = RPC_QUEUE_FULL;
    }

    vQueueDelete(rpc->response_queue);
    ESP_LOGI(TAG, "Response Queue Deleted; ID: %d\n", id);
    return res;
}

nl_err_t vault_init(vault_t *vault){
    /* Secure allocates space for vault, and checks if it exists in NVS
     *   Returns E_SUCCESS if vault found in NVS
     *   Returns E_FAILURE if vault is not found in NVS
     *
     * Storage Security Notes:
     *     * make sure seed and attempt counter are on same page
     *          - Prevents a malicious person from reseting the counter via
     *            page wipe. However, a missing attempt counter should trigger
     *            a factory reset. A page can hold 126*32 bytes
	 *     * Increment attempt counter in NVS before testing
     */
	nvs_handle nvs_secret;
    esp_err_t err;
    nl_err_t res;
    size_t required_size;

    // Allocate guarded space on the heap for the vault
    vault = sodium_malloc(sizeof(vault_t));
    if( NULL==vault ){
        ESP_LOGE(TAG, "Unable to allocate space for the Vault");
        esp_restart();
    }
    sodium_mprotect_readwrite(vault);
    sodium_memzero(vault, sizeof(vault_t));
    sodium_mprotect_noaccess(vault);

    // Initializes the secret nvs
    ESP_LOGI(TAG, "Opening SECRET namespace to check if encrypted mnemonic exists.");
    init_nvm_namespace(&nvs_secret, "secret");
    
    // Checks if mnemonic exists in NVS
    err = nvs_get_blob(nvs_secret, "mnemonic", NULL, &required_size);
    if ( ESP_OK == err){
        // vault was found and successfully read from memory;
        res = E_SUCCESS;
    }
    else{
        // Indicates the need for First-Time-Startup
        res = E_FAILURE;
    }
    
	nvs_close(nvs_secret);
    return res;
}

static bool pin_prompt(menu8g2_t *menu, vault_t *vault){
    /* Reads and Decrypts mnemonic from NVS.
     * Returns true if the vault object is now valid.
     * Will factory reset when max_attempts is reached.
     * Returns False when user presses back. */
    esp_err_t err;
    uint8_t pin_attempts;
    nvs_handle nvs_secret;
    char title[20];
    uint256_t pin_hash;
    uint256_t nonce = {0};
    int8_t decrypt_result;
    CONFIDENTIAL unsigned char enc_mnemonic[
            crypto_secretbox_MACBYTES + MNEMONIC_BUF_LEN];
    size_t required_size = sizeof(enc_mnemonic);

    if( vault->valid ){
        ESP_LOGI(TAG, "Vault is valid, skipping pin.");
        return true;
    }
    else{
        ESP_LOGI(TAG, "Vault is invalid, prompting user for pin.");
    }

    ESP_LOGI(TAG, "Opening SECRET namespace to load encrypted vault.");
    init_nvm_namespace(&nvs_secret, "secret");

    err = nvs_get_u8(nvs_secret, "pin_attempts", &pin_attempts);
    if(ESP_OK != err || pin_attempts >= CONFIG_JOLT_DEFAULT_MAX_ATTEMPT){
        factory_reset();
    }
    err = nvs_get_blob(nvs_secret, "mnemonic", enc_mnemonic, &required_size);

    for(;;){
        if(pin_attempts >= CONFIG_JOLT_DEFAULT_MAX_ATTEMPT){
            factory_reset();
        }
        sprintf(title, "Enter Pin (%d/%d)", pin_attempts+1,
                CONFIG_JOLT_DEFAULT_MAX_ATTEMPT);
        if(!pin_entry(menu, pin_hash, title)){
            // User cancelled vault operation
            nvs_close(nvs_secret);
            return false;
        };
        pin_attempts++;
        nvs_set_u8(nvs_secret, "pin_attempts", pin_attempts);
        nvs_commit(nvs_secret);

        loading_enable();
        loading_text_title("Decrypting", TITLE);
        sodium_mprotect_readwrite(vault);
        decrypt_result = crypto_secretbox_open_easy(
                (unsigned char *)(vault->mnemonic),
                enc_mnemonic, required_size, nonce, pin_hash);
        sodium_mprotect_readonly(vault);
        loading_disable();

        if(decrypt_result == 0){ //success
            sodium_memzero(enc_mnemonic, sizeof(enc_mnemonic));
            nvs_set_u8(nvs_secret, "pin_attempts", 0);
            nvs_commit(nvs_secret);
            nl_mnemonic_to_master_seed(vault->master_seed,
                    vault->mnemonic, "");
            vault->valid = true;
            ESP_LOGI(TAG, "Mnemonic successfully decrypted.");
            break;
        }
        else{
            menu8g2_display_text_title(menu, "Wrong PIN", TITLE);
        }
    }
    nvs_close(nvs_secret);
    return true;
}

void vault_task(void *vault_in){
    /* This task should be ran at HIGHEST PRIORITY
     * This task is essentially a daemon that is the only activity that should
     * be accessing the vault. This task will respond to commands that
     * request some task to be complete 
     *
     * vault must already be initialized before starting this task
     * */

    vault_t *vault = (vault_t *)vault_in;

    vault_rpc_t *cmd;
    vault_rpc_response_t response;

    menu8g2_t menu;
    menu8g2_init(&menu, &u8g2, input_queue, disp_mutex, NULL, NULL);

    /* The vault_queue holds a pointer to the vault_rpc_t object declared
     * by the producer task. Results are directly modified on that object.
     * A response status is sent back on the queue in the vault_rpc_t stating
     * the RPC error code. */
    vault_queue = xQueueCreate( CONFIG_JOLT_VAULT_RPC_QUEUE_LEN, sizeof( vault_rpc_t* ) );

    for(;;){
    	if( xQueueReceive(vault_queue, &cmd,
                pdMS_TO_TICKS(CONFIG_JOLT_DEFAULT_TIMEOUT_S * 1000)) ){
            sodium_mprotect_readonly(vault);

            // Prompt user for Pin if necessary
            if(pin_prompt(&menu, vault)){
                // Perform RPC command
                /* MASTER RPC SWITCH STATEMENT */
                switch(cmd->type){
                    case SYSCORE_START ... SYSCORE_END:
                        response = rpc_syscore(vault, cmd, &menu);
                    case NANO_START ... NANO_END:
                        response = rpc_nano(vault, cmd, &menu);
                        break;
                    default:
                        response = RPC_UNDEFINED;
                        break;
                }
            }
            else{
                response = RPC_CANCELLED;
            }
            statusbar_draw_enable = true;

            // Send back response
            xQueueOverwrite(cmd->response_queue, &response);
        }
        else{
            // Timed Out: Wipe the vault!
            sodium_mprotect_readwrite(vault);
            sodium_memzero(vault, sizeof(vault_t));
            ESP_LOGI(TAG, "Vault timed out; wiping.");
        }
        // Always disable access when outside of this task
        sodium_mprotect_noaccess(vault);
    }
}
