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


static void sign_block(){
}

static void derive_address(){
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
    err = nvs_get_blob(nvs_secret, "vault", NULL, &required_size);
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

    TickType_t xNextWakeTime = xTaskGetTickCount();
    vault_rpc_t cmd;
    bool command_cancelled = false;
    nl_err_t err;

    menu8g2_t menu;
    menu8g2_init(&menu, &u8g2, input_queue);

    sodium_mprotect_readonly(vault);

    for(;;){
    	if( xQueueReceive(vault_queue, &cmd,
                pdMS_TO_TICKS(CONFIG_NANORAY_DEFAULT_TIMEOUT_S * 1000)) ){
            // Prompt user for Pin
            if(!(vault->valid)){
                uint8_t pin_attempts;
                nvs_handle nvs_secret;
                char title[20];
                uint256_t pin_hash;
	            uint256_t nonce = {0};
                int8_t decrypt_result;
                size_t required_size;
	            CONFIDENTIAL unsigned char enc_vault[crypto_secretbox_MACBYTES +
                        sizeof(vault_t)];

                init_nvm_namespace(&nvs_secret, "secret");
                err = nvs_get_u8(nvs_secret, "pin_attempts", &pin_attempts);
                if(ESP_OK != err || pin_attempts >= CONFIG_NANORAY_DEFAULT_MAX_ATTEMPT){
                    factory_reset();
                }
                nvs_get_blob(nvs_secret, "enc_vault", enc_vault, &required_size);
                for(;;){
                    if(pin_attempts >= CONFIG_NANORAY_DEFAULT_MAX_ATTEMPT){
                        factory_reset();
                    }
                    sprintf(title, "Enter Pin (%d/%d)", pin_attempts+1,
                            CONFIG_NANORAY_DEFAULT_MAX_ATTEMPT);
                    if(!pin_entry(&menu, pin_hash, title)){
                        // User cancelled vault operation
                        command_cancelled =  true;
                        break;
                    };
                    pin_attempts++;
                    nvs_set_u8(nvs_secret, "pin_attempts", pin_attempts);
                    nvs_commit(nvs_secret);

                    sodium_mprotect_readwrite(vault);
                    decrypt_result = crypto_secretbox_open_easy(
                            (unsigned char *)&vault,
                            enc_vault, crypto_secretbox_MACBYTES + sizeof(vault_t),
                            nonce, pin_hash);
                    if(decrypt_result == 0){ //success
                        sodium_memzero(enc_vault, sizeof(enc_vault));
                        nvs_set_u8(nvs_secret, "pin_attempts", 0);
                        nvs_commit(nvs_secret);
                        break;
                    }
                }
                nvs_close(nvs_secret);
            }

            // Abort command if user pressed back
            if(command_cancelled){
                continue;
            }

            // Perform command
            switch(cmd.type){
                case(BLOCK_SIGN):
                    break;
                case(PUBLIC_KEY):
                    break;
                default:
                    break;
            }
        }
        else{
            // Timed Out: Wipe the vault!
            sodium_mprotect_readwrite(vault);
            sodium_memzero(vault, sizeof(vault_t));
        }
        // Always disable access when outside of this task
        sodium_mprotect_noaccess(vault);
    }
}
