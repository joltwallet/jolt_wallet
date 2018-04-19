#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "nvs.h"
#include "nano_lib.h"

/* Two main NVS Namespaces: ("secret", "user")
 */

static void factory_reset(){
    /* Erases all NVM and resets device */
    nvs_handle h;
    init_nvm_namespace(&h, "secret")
    nvs_erase_all(h);
    nvs_commit(h);
    nvs_close(h);

    init_nvm_namespace(&h, "user")
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
    ESP_LOGI("Opening Non-Volatile Storage (NVS) handle... ");

    err = nvs_open(namespace, NVS_READWRITE, nvs_h);
    if (err != ESP_OK) {
        ESP_LOGE("Error (%d) opening NVS handle with namespace %s!\n", err, namespace);
        return E_FAILURE;
    } else {
        ESP_LOGI("Successfully opened NVM with namespace %s\n", namespace);
        return E_SUCCESS;
    }
}

nl_err_t vault_init(){
    /* Storage Security Notes:
     *     * make sure seed and attempt counter are on same page
     *          - Prevents a malicious person from reseting the counter via
     *            page wipe. However, a missing attempt counter should trigger
     *            a factory reset. A page can hold 126*32 bytes
	 *     * Increment attempt counter in NVS before testing
     */
	nvs_handle nvs_secret;
    esp_err_t err;
    nl_err_t res;
    vault_t *vault; // Global

    // Allocate guarded space on the heap for the vault
    vault = sodium_malloc(sizeof(vault_t));
    if( NULL==vault ){
        ESP_LOGE("Unable to allocate space for the Vault");
        esp_restart();
    }

    // Initializes the secret nvs
    init_nvm_namespace(&nvs_secret, "secret");
    
    // Reads vault from nvs into ram
    err = nvs_get_blob(nvs_secret, "vault", vault, &required_size);
    if ( ESP_OK == err){
        // vault was found and successfully read from memory;
        // nothing really need to be done here
        res = E_SUCCESS;
    }
    else{
        // Indicates the need for First-Time-Startup
        res = E_FAILURE;
    }
    
	nvs_close(nvs_secret)
    return res;
}

void vault_access_task(void *menu8g2){
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
    uint64_t timeout_us;
    TickType_t xNextWakeTime = xTaskGetTickCount();
    vault_rpc_t cmd;

    sodium_mprotect_read(vault);
    timeout_us = vault->timeout_us;

    // todo: setup COMMAND_QUEUE
    for(;;){
        // todo: replace COMMAND_QUEUE placeholder (make it global)
    	if(xQueueReceive(COMMAND_QUEUE, &cmd,
                    pdMS_TO_TICKS(timeout_us / 1000))){
            sodium_mprotect_read(vault)
            // todo: check vault validity
            // todo: prompt user for pin if invalid
            // todo: Perform command
        }
        else{
            // Timedout: Wipe the vault!
            sodium_mprotect_write(vault);
            sodium_memzero(vault, sizeof(vault_t));
        }
        // Always disable access when outside of this task
        sodium_mprotect_noaccess(vault);
    }
}
