/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sodium.h"
#include <esp_system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../globals.h"
#include "../gui/entry.h"
#include "../gui/gui.h"
#include "../helpers.h"
#include "../vault.h"
#include "bipmnemonic.h"
#include "jolttypes.h"
#include "menu8g2.h"
#include "storage.h"

static const char* TAG = "storage_internal";
static const char* TITLE = "Storage Access";

jolt_err_t init_nvs_namespace(nvs_handle *nvs_h, const char *namespace) {
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    
    // Open
    err = nvs_open(namespace, NVS_READWRITE, nvs_h);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%d) opening NVS handle with namespace %s!", err, namespace);
        esp_restart();
        return E_FAILURE;
    } else {
        ESP_LOGI(TAG, "Successfully opened NVS with namespace %s!", namespace);
        return E_SUCCESS;
    }
}


bool storage_internal_exists_mnemonic() {
    /* Returens true if mnemonic exists, false otherwise */
    bool res;
    size_t required_size;
    nvs_handle nvs;
    init_nvs_namespace(&nvs, "secret");
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
            crypto_secretbox_MACBYTES + sizeof(uint256_t)];
    uint256_t nonce = {0};

    crypto_secretbox_easy(enc_bin, (unsigned char *) bin, 
            sizeof(uint256_t), nonce, pin_hash);

    // Save everything to NVS
    nvs_handle h;
    init_nvs_namespace(&h, "secret");
    nvs_erase_all(h);
    nvs_set_blob(h, "mnemonic", enc_bin, sizeof(enc_bin));
    nvs_set_u8(h, "pin_attempts", 0);
    nvs_commit(h);
    nvs_close(h);
    sodium_memzero(enc_bin, sizeof(enc_bin));
    return;
}

bool storage_internal_get_mnemonic(uint256_t mnemonic, uint256_t pin_hash) {
    /* returns 256-bit mnemonic from storage 
     * Returns true if mnemonic is decrypted successfully;
     * false if user provided pin_hash was incorrect.
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
            crypto_secretbox_MACBYTES + sizeof(uint256_t)];
    size_t required_size = sizeof(enc_mnemonic);
    uint256_t nonce = {0};

    ESP_LOGI(TAG, "Opening [secret] namespace to load encrypted mnemonic.");
    storage_get_blob(enc_mnemonic, &required_size, "secret", "mnemonic");
    uint8_t decrypt_result = crypto_secretbox_open_easy( (unsigned char *)mnemonic,
            enc_mnemonic, required_size, nonce, pin_hash);
    sodium_memzero(enc_mnemonic, sizeof(enc_mnemonic));
    return 0 == decrypt_result;
}

uint32_t storage_internal_get_pin_count() {
    /* Gets the current PIN counter. This value is monotomicly increasing. 
     * Factory resets on failure to retrieve pin_count value. */
    uint32_t pin_count;
    if( !storage_get_u32(&pin_count, "secret", "pin_count", UINT32_MAX) ) {
        storage_factory_reset();
    }
    return pin_count;
}

void storage_internal_set_pin_count(uint32_t count) {
    if( !storage_set_u32(count, "secret", "pin_count") ) {
        storage_factory_reset();
    }
}

uint32_t storage_internal_get_pin_last() {
    /* Gets the pin counter of the last successful pin attempt.
     */
    uint32_t pin_last;
    if( !storage_get_u32(&pin_last, "secret", "pin_last", 0) ) {
        storage_factory_reset();
    }
    return pin_last;
}

void storage_internal_set_pin_last(uint32_t count) {
    if( !storage_set_u32(count, "secret", "pin_last") ) {
        storage_factory_reset();
    }
}

bool storage_internal_get_u8(uint8_t *value, char *namespace, char *key,
        uint8_t default_value ) {
    bool res;
    nvs_handle nvs;
    esp_err_t err;

    init_nvs_namespace(&nvs, namespace);
    err = nvs_get_u8(nvs, key, value);
    nvs_close(nvs);

    if ( ESP_OK==err ) {
        res = true;
    } 
    else {
        *value = default_value;
        res = false;
    }

    return res;
}

bool storage_internal_set_u8(uint8_t value, char *namespace, char *key) {
    nvs_handle nvs;
    esp_err_t err;

    init_nvs_namespace(&nvs, namespace);
    err = nvs_set_u8(nvs, key, value);
    nvs_close(nvs);

    return ESP_OK==err;
}

bool storage_internal_get_u16(uint16_t *value, char *namespace, char *key,
        uint16_t default_value ) {
    bool res;
    nvs_handle nvs;
    esp_err_t err;

    init_nvs_namespace(&nvs, namespace);
    err = nvs_get_u16(nvs, key, value);
    nvs_close(nvs);

    if ( ESP_OK==err ) {
        res = true;
    } 
    else {
        *value = default_value;
        res = false;
    }

    return res;
}

bool storage_internal_set_u16(uint16_t value, char *namespace, char *key) {
    nvs_handle nvs;
    esp_err_t err;

    init_nvs_namespace(&nvs, namespace);
    err = nvs_set_u16(nvs, key, value);
    nvs_close(nvs);

    return ESP_OK==err;
}

bool storage_internal_get_u32(uint32_t *value, char *namespace, char *key,
        uint32_t default_value ) {
    bool res;
    nvs_handle nvs;
    esp_err_t err;

    init_nvs_namespace(&nvs, namespace);
    err = nvs_get_u32(nvs, key, value);
    nvs_close(nvs);

    if ( ESP_OK==err ) {
        res = true;
    } 
    else {
        *value = default_value;
        res = false;
    }

    return res;
}

bool storage_internal_set_u32(uint32_t value, char *namespace, char *key) {
    nvs_handle nvs;
    esp_err_t err;

    init_nvs_namespace(&nvs, namespace);
    err = nvs_set_u32(nvs, key, value);
    nvs_close(nvs);

    return ESP_OK==err;
}


bool storage_internal_get_str(char *buf, size_t *required_size,
        char *namespace, char *key, char *default_value) {
    bool res;
    nvs_handle nvs;
    init_nvs_namespace(&nvs, namespace);
    esp_err_t err;
    err = nvs_get_str(nvs, key, buf, required_size);
    nvs_close(nvs);
    if ( ESP_OK==err ) {
        res = true;
    } 
    else if ( ESP_ERR_NVS_NOT_FOUND==err ) {
        if( NULL==buf ) {
            if( NULL != default_value ) {
                *required_size = strlen(default_value) + 1;
            }
            else {
                *required_size = 1;
            }
        }
        else {
            if( NULL != default_value ) {
                strcpy(buf, default_value);
            }
            else {
                *buf = '\0';
            }
        }
        res = false;
    }
    else {
        res = false;
    }
    return res;
}

bool storage_internal_set_str(char *str, char *namespace, char *key) {
    nvs_handle nvs;
    esp_err_t err;

    init_nvs_namespace(&nvs, namespace);
    err = nvs_set_str(nvs, key, str);
    nvs_close(nvs);

    return ESP_OK==err;
}

bool storage_internal_get_blob(unsigned char *buf, size_t *required_size,
        char *namespace, char *key) {
    nvs_handle nvs;
    init_nvs_namespace(&nvs, namespace);
    esp_err_t err;
    err = nvs_get_blob(nvs, key, buf, required_size);
    nvs_close(nvs);
    return ESP_OK==err;
}

bool storage_internal_set_blob(unsigned char *buf, size_t len,
        char *namespace, char *key) {
    nvs_handle nvs;
    esp_err_t err;

    init_nvs_namespace(&nvs, namespace);
    err = nvs_set_blob(nvs, key, buf, len);
    nvs_close(nvs);

    return ESP_OK==err;
}


void storage_internal_factory_reset() {
    // todo: fix this up
    nvs_handle h;

    init_nvs_namespace(&h, "secret");
    nvs_erase_all(h);
    nvs_commit(h);
    nvs_close(h);

    init_nvs_namespace(&h, "user");
    nvs_erase_all(h);
    nvs_commit(h);
    nvs_close(h);

    init_nvs_namespace(&h, "nano");
    nvs_erase_all(h);
    nvs_commit(h);
    nvs_close(h);

    esp_restart();
}

bool storage_internal_erase_key(char *namespace, char *key) {
    nvs_handle nvs;
    esp_err_t err;

    init_nvs_namespace(&nvs, namespace);
    err = nvs_erase_key(nvs, key);
    nvs_close(nvs);

    return ESP_OK==err;
}
