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
#include "security.h"
#include "globals.h"

/* Two main NVS Namespaces: ("secret", "user") */
static const char* TAG = "security";
static void nvs_log_err(esp_err_t err);

static void factory_reset(){
    /* Erases all NVM and resets device */
    nvs_handle h;

    init_nvm_namespace(&h, "secret");
    nvs_erase_all(h);
    nvs_commit(h);
    nvs_close(h);

    init_nvm_namespace(&h, "user");
    nvs_erase_all(h);
    nvs_commit(h);
    nvs_close(h);

    esp_restart();
}

static vault_rpc_response_t rpc_public_key(vault_t *vault, vault_rpc_t *cmd){
    // Derive private key from mnemonic
    vault_rpc_response_t response;
    CONFIDENTIAL uint256_t private_key;
    nl_master_seed_to_nano_private_key(private_key, 
            vault->master_seed,
            cmd->payload.public_key.index);
    // Derive public key from private
    nl_private_to_public(cmd->payload.public_key.block.account, private_key);
    sodium_memzero(private_key, sizeof(private_key));
    response = RPC_SUCCESS;
    return response;
}

static void nvs_log_err(esp_err_t err){
    switch(err){
        case(ESP_OK): ESP_LOGI(TAG, "ESP_OK"); break;
        case(ESP_ERR_NVS_NOT_INITIALIZED): ESP_LOGI(TAG, "ESP_ERR_NVS_NOT_INITIALIZED "); break;
        case(ESP_ERR_NVS_NOT_FOUND): ESP_LOGI(TAG, "ESP_ERR_NVS_NOT_FOUND"); break;
        case(ESP_ERR_NVS_TYPE_MISMATCH): ESP_LOGI(TAG, "ESP_ERR_NVS_TYPE_MISMATCH"); break;
        case(ESP_ERR_NVS_READ_ONLY): ESP_LOGI(TAG, "ESP_ERR_NVS_READ_ONLY"); break;
        case(ESP_ERR_NVS_NOT_ENOUGH_SPACE): ESP_LOGI(TAG, "ESP_ERR_NVS_NOT_ENOUGH_SPACE"); break;
        case(ESP_ERR_NVS_INVALID_NAME): ESP_LOGI(TAG, "ESP_ERR_NVS_INVALID_NAME"); break;
        case(ESP_ERR_NVS_INVALID_HANDLE): ESP_LOGI(TAG, "ESP_ERR_NVS_INVALID_HANDLE"); break;
        case(ESP_ERR_NVS_REMOVE_FAILED): ESP_LOGI(TAG, "ESP_ERR_NVS_REMOVE_FAILED"); break;
        case(ESP_ERR_NVS_KEY_TOO_LONG): ESP_LOGI(TAG, "ESP_ERR_NVS_KEY_TOO_LONG"); break;
        case(ESP_ERR_NVS_PAGE_FULL): ESP_LOGI(TAG, "ESP_ERR_NVS_PAGE_FULL"); break;
        case(ESP_ERR_NVS_INVALID_STATE): ESP_LOGI(TAG, "ESP_ERR_NVS_INVALID_STATE"); break;
        case(ESP_ERR_NVS_INVALID_LENGTH): ESP_LOGI(TAG, "ESP_ERR_NVS_INVALID_LENGTH"); break;
        case(ESP_ERR_NVS_NO_FREE_PAGES): ESP_LOGI(TAG, "ESP_ERR_NVS_NO_FREE_PAGES"); break;
        case(ESP_ERR_NVS_VALUE_TOO_LONG): ESP_LOGI(TAG, "ESP_ERR_NVS_VALUE_TOO_LONG"); break;
        case(ESP_ERR_NVS_PART_NOT_FOUND): ESP_LOGI(TAG, "ESP_ERR_NVS_PART_NOT_FOUND"); break;
        case(ESP_ERR_INVALID_ARG): ESP_LOGI(TAG, "ESP_ERR_INVALID_ARG"); break;
    }
}

nl_err_t init_nvm_namespace(nvs_handle *nvs_h, const char *namespace){
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    
    // Open
    ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle... ");

    err = nvs_open(namespace, NVS_READWRITE, nvs_h);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%d) opening NVS handle with namespace %s!\n", err, namespace);
        return E_FAILURE;
    } else {
        ESP_LOGI(TAG, "Successfully opened NVM with namespace %s\n", namespace);
        return E_SUCCESS;
    }
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

    // Initializes the secret nvs
    init_nvm_namespace(&nvs_secret, "secret");
    
    // Reads vault from nvs into ram
    err = nvs_get_blob(nvs_secret, "mnemonic", NULL, &required_size);
    if ( ESP_OK == err){
        // vault was found and successfully read from memory;
        // nothing really need to be done here
        res = E_SUCCESS;
    }
    else{
        // Indicates the need for First-Time-Startup
        res = E_FAILURE;
    }
    
	nvs_close(nvs_secret);
    return res;
}

static bool pin_prompt(vault_t *vault){
    /* Reads and Decrypts mnemonic from NVS.
     * Returns true if the vault object is now valid.
     * Will factory reset when max_attempts is reached.
     * Returns False when user presses back. */
    nl_err_t err;
    uint8_t pin_attempts;
    nvs_handle nvs_secret;
    char title[20];
    uint256_t pin_hash;
    uint256_t nonce = {0};
    int8_t decrypt_result;
    CONFIDENTIAL unsigned char enc_mnemonic[crypto_secretbox_MACBYTES + MNEMONIC_BUF_LEN];
    size_t required_size = sizeof(enc_mnemonic);
    menu8g2_t menu;
    menu8g2_init(&menu, &u8g2, input_queue);

    if( vault->valid ){
        return true;
    }

    init_nvm_namespace(&nvs_secret, "secret");
    err = nvs_get_u8(nvs_secret, "pin_attempts", &pin_attempts);
    if(ESP_OK != err || pin_attempts >= CONFIG_NANORAY_DEFAULT_MAX_ATTEMPT){
        factory_reset();
    }
    err = nvs_get_blob(nvs_secret, "mnemonic", enc_mnemonic, &required_size);
    nvs_log_err(err);

    for(;;){
        if(pin_attempts >= CONFIG_NANORAY_DEFAULT_MAX_ATTEMPT){
            factory_reset();
        }
        sprintf(title, "Enter Pin (%d/%d)", pin_attempts+1,
                CONFIG_NANORAY_DEFAULT_MAX_ATTEMPT);
        if(!pin_entry(&menu, pin_hash, title)){
            // User cancelled vault operation
            nvs_close(nvs_secret);
            return false;
        };
        pin_attempts++;
        nvs_set_u8(nvs_secret, "pin_attempts", pin_attempts);
        nvs_commit(nvs_secret);

        sodium_mprotect_readwrite(vault);
        decrypt_result = crypto_secretbox_open_easy(
                (unsigned char *)(vault->mnemonic),
                enc_mnemonic, required_size, nonce, pin_hash);
        if(decrypt_result == 0){ //success
            sodium_memzero(enc_mnemonic, sizeof(enc_mnemonic));
            nvs_set_u8(nvs_secret, "pin_attempts", 0);
            nvs_commit(nvs_secret);
            nl_mnemonic_to_master_seed(vault->master_seed,
                    vault->mnemonic, "");
            vault->valid = true;
            break;
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

    // Checks last time vault has been accessed
    // If not accessed recently, prompt for pin
    // If accessed recently, update the last accessed time
    // ONLY THIS FUNCTION SHOULD MODIFY SODIUM_MPROTECT FOR VAULT
    vault_t *vault = (vault_t *)vault_in;

    vault_rpc_t *cmd;
    vault_rpc_response_t response;

    menu8g2_t menu;
    menu8g2_init(&menu, &u8g2, input_queue);

    /* The vault_queue holds a pointer to the vault_rpc_t object declared
     * by the producer task. Results are directly modified on that object.
     * A response status is sent back on the queue in the vault_rpc_t stating
     * the RPC error code. */
    vault_queue = xQueueCreate( 4, sizeof( vault_rpc_t* ) );

    for(;;){
    	if( xQueueReceive(vault_queue, &cmd,
                pdMS_TO_TICKS(CONFIG_NANORAY_DEFAULT_TIMEOUT_S * 1000)) ){
            sodium_mprotect_readonly(vault);

            // Prompt user for Pin if necessary
            if(!pin_prompt(vault)){
                continue;
            }

            // Perform command
            switch(cmd->type){
                case(BLOCK_SIGN):
                    break;
                case(PUBLIC_KEY):{
                    response = rpc_public_key(vault, cmd);
                    break;
                }
                case(FACTORY_RESET):
                    factory_reset();
                    break;
                default:
                    break;
            }

            // Send back response
            xQueueSend(cmd->response_queue, &response, 0);
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
