/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_log.h"
#include "sodium.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "menu8g2.h"
#include "jolttypes.h"
#include "bipmnemonic.h"
#include "helpers.h"
#include "globals.h"
#include "vault.h"
#include "gui/gui.h"
#include "gui/entry.h"
#include "gui/statusbar.h"
#include "gui/loading.h"

static const char* TAG = "vault";
static const char* TITLE = "Vault Access";

vault_t *vault = NULL;
/* The following semaphores are not part of the vault object so we can use
 * sodium_malloc() */
static SemaphoreHandle_t vault_sem; // Used for general vault access
static SemaphoreHandle_t vault_watchdog_sem; // Used to kick the dog

void vault_sem_take() {
    /* Takes Vault semaphore; restarts device if timesout during take. */
    if( xSemaphoreTake(vault_sem, pdMS_TO_TICKS(CONFIG_JOLT_TIMEOUT_TIMEOUT_MS) ) ) {
        // Timed out trying to take the semaphore; reset the device
        // And let the bootloader wipe the RAM
        esp_restart();
    }
}

void vault_sem_give() {
    xSemaphoreGive(vault_sem);
}

static void vault_watchdog_task() {
    /* Daemon-like task to wipe vault after a configurable timeout.
     * Should be given very high priority to prevent deadlocks. */
	for(;;) {
        if( xSemaphoreTake( vault_watchdog_sem, 
				pdMS_TO_TICKS(CONFIG_JOLT_DEFAULT_TIMEOUT_S * 1000)) ) {
            // Command to reset the private node watchdog;
            // Note, vault_sem is probably taken if in here
            // Do Nothing
        }
        else {
            // WatchDog timeout; wipe the private node
            vault_clear();
        }
	}
}

bool vault_setup() {
    /* Creates the private key object and the watchdog task to monitor it.
     * Also checks to see if there is a stored secret or not.
     *
     * Returns True if a secret (mnemonic) has been previously setup.
     * Returns False if no secret (mnemonic) has been setup.*/
    vault = sodium_malloc(sizeof(vault_t));
    if( NULL==vault ){
        ESP_LOGE(TAG, "Unable to allocate space for the Vault");
        esp_restart();
    }
    sodium_mprotect_readwrite(vault);
    sodium_memzero(vault, sizeof(vault_t));
    sodium_mprotect_readonly(vault);
    vault_sem = xSemaphoreCreateMutex();
    vault_watchdog_sem = xSemaphoreCreateBinary();
    xTaskCreate(private_watchdog_task, "Vault", 32000, NULL, 16, NULL);

    // Checks if stored secret exists
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    size_t required_size;
    nvs_handle nvs_secret;
    res = ESP_OK==nvs_get_blob(nvs_secret, "mnemonic", NULL, &required_size);
    nvs_close(nvs_secret);
#elif CONFIG_JOLT_STORE_ATAES132A
    // todo: implement
#endif
    return res;
}

void vault_clear() {
    /* Clears the Vault struct.
     * Does NOT clear the following so that the node can be easily restored:
     *      * purpose
     *      * coin_type
     *      * bip32_key
     */
    vault_sem_take();
    ESP_LOGI(TAG, "Clearing Vault.");
    sodium_mprotect_readwrite(vault);
    vault->valid = false;
    sodium_memzero(vault->node, sizeof(hd_node_t));
    sodium_mprotect_readonly(vault);
    ESP_LOGI(TAG, "Clearing Vault Complete.");
    vault_sem_give();
}

bool vault_set(uint32_t purpose, uint32_t coin_type, const char *bip32_key) {
    // To only be called before launching an app, or when changing firmware
    // settings.
    //
    // Inside this function (really inside get_master_seed()),
    // the GUI is invoked for PIN and Passphrase.
    //
    // Returns true if successfully set
    // Returns false if not set (user cancellation)
    CONFIDENTIAL uint512_t master_seed;
    bool res;
    // Inside get_master_seed(), PIN and passphrase are prompted for
    if(!get_master_seed(master_seed)) {
        res = false;
        goto exit;
    }
    strlcpy( vault->bip32_key, bip32_key, sizeof(vault->bip32_key) );
    vault->purpose = purpose;
    vault->coin_type = coin_type;
    bm_master_seed_to_node(vault->node, master_seed, vault->bip32_key,
            2, vault->purpose, vault->coin_type);
    res = true;

exit:
    sodium_memzero(master_seed, sizeof(master_seed));
    return res;
}

bool refresh_vault() {
    /* Kicks dog if vault is valid.
     * Repopulates node (therefore prompting user for PIN/Passphrase otherwise
     *
     * To be called within an app right before a private key is to be used.
     *
     * Returns true on success,
     * false if user cancels (if node needs restored)
     */
    bool res;
    vault_sem_take();
    if( vault->valid ) {
        // Kick the dog
        xSemaphoreGive(vault_watchdog_sem);
        vault_sem_give();
        return true;
    }
    else {
        // Give up semaphore while prompt using for PIN/Passphrase
        vault_sem_give();
        // Inside get_master_seed(), PIN and passphrase are prompted for
        CONFIDENTIAL uint512_t master_seed;
        if(!get_master_seed(master_seed)) {
            return false;
        }

        vault_sem_take();

        // Kick the dog first to avoid a potential race condition where the 
        // watchdog resets a just-set node.
        xSemaphoreGive(vault_watchdog_sem);

        bm_master_seed_to_node(vault->node, master_seed, vault->bip32_key,
            2, vault->purpose, vault->coin_type);
        vault_sem_give();
        sodium_memzero(master_seed, sizeof(master_seed));
        return true;
    }
}

static bool get_master_seed(uint512_t master_seed) {
    // Command-level function to get the user's mnemonic
    // WILL prompt user for PIN and/or passphrase
    //
    // Internally saves/restores display since PIN/Passphrase prompt
    // overwrites screen buffer.
    //
    // Returns True if user successfully entered PIN/Passphrase
    // Returns False if user cancels
    bool pin_res;
    bool passphrase_res;
    bool res;
    CONFIDENTIAL char passphrase[BM_PASSPHRASE_BUF_LEN] = "";
    CONFIDENTIAL char mnemonic[BM_MNEMONIC_BUF_LEN];

    SCREEN_SAVE;
    
    // Populate mnemonic by prompting user for PIN
#if CONFIG_JOLT_STORE_INTERNAL
    {
    /* This implementation requires no external components, but is vulnerable
     * to having the ciphertext being restored on the SPI Flash, granting an
     * attacker unlimited PIN attempts with minimal specialty tools.
     *
     * Todos:
     *  * NVS currently doesn't use hardware encryption; either:
     *      * Wait for NVS to support encryption
     *      * Implement it ourselves
     *  * Save 256-bits and convert it upon load instead of storing and loading 
     *    the whole mnemonic string
     */
    jolt_err_t err;
    CONFIDENTIAL unsigned char enc_mnemonic[
            crypto_secretbox_MACBYTES + BM_MNEMONIC_BUF_LEN];
    size_t required_size = sizeof(enc_mnemonic);

    ESP_LOGI(TAG, "Opening SECRET namespace to load encrypted mnemonic.");
    nvs_handle nvs_secret;
    init_nvm_namespace(&nvs_secret, "secret");
    uint8_t pin_attempts;
    err = nvs_get_u8(nvs_secret, "pin_attempts", &pin_attempts);
    if( ESP_OK != err || pin_attempts >= CONFIG_JOLT_DEFAULT_MAX_ATTEMPT ) {
        factory_reset();
    }
    err = nvs_get_blob(nvs_secret, "mnemonic", enc_mnemonic, &required_size);

    for(;;) { // Loop will exit upon successful PIN or cancellation
        if( pin_attempts >= CONFIG_JOLT_DEFAULT_MAX_ATTEMPT ) {
            factory_reset();
        }
        char title[20];
        sprintf(title, "Enter Pin (%d/%d)", pin_attempts+1,
                CONFIG_JOLT_DEFAULT_MAX_ATTEMPT);
        uint256_t pin_hash;
        if( !entry_pin(menu, pin_hash, title) ) {
            // User cancelled vault operation
            nvs_close(nvs_secret);
            res = false;
            goto exit;
        };
        pin_attempts++;
        nvs_set_u8(nvs_secret, "pin_attempts", pin_attempts);
        nvs_commit(nvs_secret);

        loading_enable();
        loading_text_title("Decrypting", TITLE);
        decrypt_result = crypto_secretbox_open_easy( (unsigned char *)mnemonic,
                enc_mnemonic, required_size, nonce, pin_hash);
        loading_disable();

        if(decrypt_result == 0){ // Success
            sodium_memzero(enc_mnemonic, sizeof(enc_mnemonic));
            nvs_set_u8(nvs_secret, "pin_attempts", 0);
            nvs_commit(nvs_secret);
            res = true;
            ESP_LOGI(TAG, "Mnemonic successfully decrypted.");
            break;
        }
        else{
            menu8g2_display_text_title(menu, "Wrong PIN", TITLE);
        }
    }
    nvs_close(nvs_secret);
    }
#elif CONFIG_JOLT_STORE_ATAES132A
    // todo: implement
#endif

    // todo: fetch passphrase
    strlcpy(passphrase, "", sizeof(passphrase)); // dummy placeholder

    // Derive master seed
    res = bm_mnemonic_to_master_seed(master_seed, mnemonic, passphrase);

exit:
    sodium_memzero(mnemonic, sizeof(mnemonic));
    sodium_memzero(passphrase, sizeof(passphrase));

    SCREEN_RESTORE;
    return res;
}
