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

static const char* TAG = "storage_hal";
static const char* TITLE = "Storage Access";

bool storage_internal_exists_mnemonic() {
    /* Returens true if mnemonic exists, false otherwise */
    size_t required_size;
    nvs_handle nvs;
    res = ESP_OK==nvs_get_blob(nvs, "mnemonic", NULL, &required_size);
    nvs_close(nvs);
    return res;
}

void storage_internal_set_mnemonic(uint256_t bin, uint256_t pin_hash) {
    // encrypt; only purpose is to reduce mnemonic redundancy to make a
    // frozen data remanence attack infeasible. Also convenient pin
    // checking. Nonce is irrelevant for this encryption.
    //
    // Also resets the pin_attempts counter
    CONFIDENTIAL unsigned char enc_bin[
            crypto_secretbox_MACBYTES + sizeof(bin)];
    uint256_t nonce = {0};

    crypto_secretbox_easy(enc_bin, (unsigned char *) bin, 
            sizeof(bin), nonce, pin_hash);

    // Save everything to NVS
    nvs_handle h;
    init_nvm_namespace(&h, "secret");
    nvs_erase_all(h);
    nvs_set_blob(h, "mnemonic", enc_bin, sizeof(enc_bin));
    nvs_set_u8(h, "pin_attempts", 0);
    nvs_commit(h);
    nvs_close(h);
    sodium_memzero(enc_bin, sizeof(enc_bin));
    return;
}

bool storage_internal_get_mnemonic(uint256_t mnemonic) {
    /* prompt user for pin, returns 256-bit mnemonic from storage 
     * Returns true if mnemonic is returned; false if user cancelled
     * 
     * This implementation requires no external components, but is vulnerable
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
    CONFIDENTIAL unsigned char enc_mnemonic[
            crypto_secretbox_MACBYTES + BM_MNEMONIC_BUF_LEN];
    size_t required_size = sizeof(enc_mnemonic);

    ESP_LOGI(TAG, "Opening SECRET namespace to load encrypted mnemonic.");
    nvs_handle nvs;
    init_nvm_namespace(&nvs, "secret");
    uint8_t pin_attempts;
    if( ESP_OK != nvs_get_u8(nvs, "pin_attempts", &pin_attempts) ||
            pin_attempts >= CONFIG_JOLT_DEFAULT_MAX_ATTEMPT ) {
        factory_reset();
    }
    nvs_get_blob(nvs, "mnemonic", enc_mnemonic, &required_size);

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
            nvs_close(nvs);
            res = false;
            goto exit;
        };
        pin_attempts++;
        nvs_set_u8(nvs, "pin_attempts", pin_attempts);
        nvs_commit(nvs);

        loading_enable();
        loading_text_title("Decrypting", TITLE);
        decrypt_result = crypto_secretbox_open_easy( (unsigned char *)mnemonic,
                enc_mnemonic, required_size, nonce, pin_hash);
        loading_disable();

        if(decrypt_result == 0){ // Success
            sodium_memzero(enc_mnemonic, sizeof(enc_mnemonic));
            nvs_set_u8(nvs, "pin_attempts", 0);
            nvs_commit(nvs);
            res = true;
            ESP_LOGI(TAG, "Mnemonic successfully decrypted.");
            break;
        }
        else{
            menu8g2_display_text_title(menu, "Wrong PIN", TITLE);
        }
    }
    nvs_close(nvs);

exit:
    return res;
}

uint32_t storage_internal_get_pin_count() {
    /* Gets the current PIN counter. This value is monotomicly increasing. */
    uint32_t res;
exit:
    return res;

}

void storage_internal_set_pin_count(uint32_t count) {
}

uint32_t storage_internal_get_pin_last() {
    /* Gets the pin counter of the last successful pin attempt.
     */
    uint32_t res;
exit:
    return res;

}

void storage_internal_set_pin_last(uint32_t count) {
}


bool storage_internal_get_u8(uint8_t *value, char *namespace, char *key, uint8_t *default_value ) {
    /* Populates [value] from value in storage with [namespace] and [key].
     * If [key] is not found in storage, set [value] to [default_value]
     *
     * Returns true if key was found.
     */
    bool res;
    nvs_handle nvs;
    if( E_SUCCESS == init_nvm_namespace(&nvs, namespace) ) {
        nvs_get_u8(nvs, key, &value);
        nvs_close(nvs);
    }
    else {
        res = false;
        // should probably do more here
    }

exit:
    return res;
}

bool storage_internal_set_u8(uint8_t value, char *namespace, char *key) {
    /* Stores [value] into [key]. Primarily used for settings.
     *
     * Returns true on success, false on failure.
     * todo: MACRO and function for all types*/
    bool res;
exit:
    return res;
}
