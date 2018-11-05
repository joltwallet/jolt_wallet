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

#include "jolttypes.h"
#include "bipmnemonic.h"
#include "jolt_helpers.h"
#include "jolt_globals.h"
#include "vault.h"
#include "hal/storage/storage.h"
#include "jolt_gui/jolt_gui.h"

static const char* TAG = "vault";
static const char* TITLE = "Vault Access";

static lv_action_t back_cb;
static lv_action_t enter_cb;


vault_t *vault = NULL;
/* The following semaphores are not part of the vault object so we can use
 * sodium_malloc() */
static SemaphoreHandle_t vault_sem; // Used for general vault access
static SemaphoreHandle_t vault_watchdog_sem; // Used to kick the dog


void vault_sem_take() {
    /* Takes Vault semaphore; restarts device if timesout during take. */
    if( !xSemaphoreTake(vault_sem, pdMS_TO_TICKS(CONFIG_JOLT_TIMEOUT_TIMEOUT_MS) ) ) {
        // Timed out trying to take the semaphore; reset the device
        // And let the bootloader wipe the RAM
        ESP_LOGI(TAG, "Failed taking vault semaphore. Rebooting...");
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
    /* NOTE: ESP32 doesn't properly implement the mprotect features;
     * awaiting for their implementation for additional security. */
    sodium_mprotect_readwrite(vault);
    sodium_memzero(vault, sizeof(vault_t));
    sodium_mprotect_readonly(vault);
    vault_sem = xSemaphoreCreateMutex();
    vault_watchdog_sem = xSemaphoreCreateBinary();
    // todo: tweak this memory value
    xTaskCreate(vault_watchdog_task, "VaultWatchDog", 10000, NULL, 16, NULL);

    // Checks if stored secret exists
    return storage_exists_mnemonic();
}

void vault_clear() {
    /* Clears the Vault struct.
     * Does NOT clear the following so that the node can be easily restored:
     *      * purpose
     *      * coin_type
     *      * bip32_key
     */
    if(vault->valid) {
        vault_sem_take();
        ESP_LOGI(TAG, "Clearing Vault.");
        sodium_mprotect_readwrite(vault);
        vault->valid = false;
        sodium_memzero(&(vault->node), sizeof(hd_node_t));
        sodium_mprotect_readonly(vault);
        ESP_LOGI(TAG, "Clearing Vault Complete.");
        vault_sem_give();
    }
}

// Store the callback to perform when node derivation is complete
static lv_action_t cb_vault_set_success = NULL;

static void derivation_master_seed_task(jolt_derivation_t *status) {
    bm_mnemonic_to_master_seed_progress(
            jolt_gui_store.derivation.master_seed, 
            jolt_gui_store.derivation.mnemonic,
            jolt_gui_store.derivation.passphrase,
            &(status->progress));
    status->progress = JOLT_DERIVATION_PROGRESS_DONE;
    vTaskDelete(NULL);
}

static lv_action_t post_mnemonic_to_master_seed(void *dummy) {
    vault_sem_take();
    sodium_mprotect_readwrite(vault);
    bm_master_seed_to_node(&(vault->node), 
            jolt_gui_store.derivation.master_seed, vault->bip32_key,
            2, vault->purpose, vault->coin_type);
    vault->valid = true;
    sodium_mprotect_readonly(vault);
    vault_sem_give();

    /* Clean up */
    sodium_memzero(jolt_gui_store.derivation.mnemonic,
            sizeof(jolt_gui_store.derivation.mnemonic));
    sodium_memzero(jolt_gui_store.derivation.master_seed,
            sizeof(jolt_gui_store.derivation.master_seed));
    sodium_memzero(jolt_gui_store.derivation.passphrase,
            sizeof(jolt_gui_store.derivation.passphrase));

    if( NULL != cb_vault_set_success ) {
        ESP_LOGI(TAG, "Calling Post-PIN Callback with valid vault.");
        cb_vault_set_success(NULL);
        cb_vault_set_success = NULL;
    }
    else {
        ESP_LOGE(TAG, "No Post-PIN callback set");
    }

    return LV_RES_OK;
}

/* Gets executed after a successful pin entry.
 * Populates the vault node.
 *     * Calls user success callback
 */
static lv_action_t pin_success_cb() {
    /* Pin screen just populated jolt_gui_store.derivation.mnemonic_bin */

    // todo: get passphrase here
    strlcpy(jolt_gui_store.derivation.passphrase, "",
            sizeof(jolt_gui_store.derivation.passphrase)); // dummy placeholder

    /* Convert Binary Mnemonic to String; Clear Binary */
    bm_bin_to_mnemonic(jolt_gui_store.derivation.mnemonic, sizeof(jolt_gui_store.derivation.mnemonic),
            jolt_gui_store.derivation.mnemonic_bin, 256);
    sodium_memzero(jolt_gui_store.derivation.mnemonic_bin, 
            sizeof(jolt_gui_store.derivation.mnemonic_bin));

    static jolt_derivation_t status;
    status.progress = 0;
    status.data = NULL;
    status.cb = post_mnemonic_to_master_seed; // Gets called after master_seed progress is complete
    status.scr = jolt_gui_scr_loading_create("");
    jolt_gui_scr_loading_update(status.scr, NULL, "Unlocking", 0);

    jolt_gui_progress_task_create(&status);
    xTaskCreate(derivation_master_seed_task,
            "MasterDeriv", 16000,
            (void *)&status,
            CONFIG_JOLT_TASK_PRIORITY_DERIVATION, &(status.derivation_task));

    return LV_RES_OK;
}

void vault_set(uint32_t purpose, uint32_t coin_type, const char *bip32_key,
        lv_action_t failure_cb, lv_action_t success_cb) {
    /* 
     * To only be called before launching an app, or when changing firmware
     * settings.
     * Since this *may* prompt the user for a PIN, it must be non-blocking
     * to agree with the GUI.
     *
     * If the vault is successfully set, the success callback will be executed.
     * On failure to set the vault, the failure callback will be executed.
     *
     */

    /* Don't require pin if logging into app with identical vault parameters */
    vault_sem_take();
    if( true == vault->valid
            && vault->purpose   == purpose
            && vault->coin_type == coin_type
            && 0 == strcmp(vault->bip32_key, bip32_key) ) {
        ESP_LOGI(TAG, "Vault is valid; kicking the dog.");
        xSemaphoreGive(vault_watchdog_sem);
        vault_sem_give();
        success_cb(NULL);
        return;
    }
    vault_sem_give();

    /* Populate Vault with derivation parameters */
    vault_sem_take();
    sodium_mprotect_readwrite(vault);
    vault->valid = false;
    vault->purpose = purpose;
    vault->coin_type = coin_type;
    strlcpy( vault->bip32_key, bip32_key, sizeof(vault->bip32_key) );
    sodium_mprotect_readonly(vault);
    vault_sem_give();

    /* Set success callback */
    cb_vault_set_success = success_cb;

    jolt_gui_scr_pin_create(failure_cb, pin_success_cb);
}

void vault_refresh(lv_action_t failure_cb, lv_action_t success_cb) {
    /* Kicks dog if vault is valid.
     * Repopulates node (therefore prompting user for PIN otherwise
     *
     * To be called within an app right before a private key is to be used.
     *
     * Returns true on success,
     * false if user cancels (if node needs restored)
     */
    cb_vault_set_success = success_cb;
    vault_sem_take();
    if( vault->valid ) {
        // Kick the dog
        ESP_LOGI(TAG, "Vault is valid; kicking the dog.");
        xSemaphoreGive(vault_watchdog_sem);
        vault_sem_give();
        success_cb(NULL);
    }
    else {
        vault_sem_give();
        jolt_gui_scr_pin_create(failure_cb, pin_success_cb);
    }
}
