/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */
#include "esp_log.h"
#include "sodium.h"
#include <esp_system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jolt_globals.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_helpers.h"
#include "vault.h"
#include "bipmnemonic.h"
#include "jolttypes.h"
#include "storage.h"
#include "storage_internal.h"
#include "aes132_comm_marshaling.h"
#include "aes132_jolt.h"
#include "aes132_cmd.h"

static const char* TAG = "storage_ataes132a";
static const char* TITLE = "Storage Access";

bool storage_ataes132a_startup() {
    /* Check if device is locked */
    uint8_t res;
    if( !(storage_internal_startup() && aes132_jolt_setup()) ) {
        // something bad happened; but we should never get here because
        // aes132_jolt_setup() should restart esp32 at the slightest hint
        // of error.
        return false;
    }
    else {
        return true;
    }
}

bool storage_ataes132a_exists_mnemonic() {
    /* Returens true if mnemonic exists, false otherwise */
    bool res;
    res = storage_internal_exists_mnemonic();
    return res;
}

static void xor256(uint8_t *out, uint8_t *x, uint8_t *y) {
    for(uint8_t i=0; i < 32; i ++) {
        out[i] = x[i] ^ y[i];
    }
}

static void rand256(uint8_t *buf) {
    /* Generates 256-bits of random data from esp32 and ataes132a sources */
    uint8_t res;
    CONFIDENTIAL uint256_t esp32_entropy;
    CONFIDENTIAL uint256_t aes132_entropy;
    bm_entropy256(esp32_entropy);
    res = aes132_rand(aes132_entropy, sizeof(aes132_entropy));
    if( res ) {
        esp_restart();
    }
    xor256(buf, esp32_entropy, aes132_entropy);
    sodium_memzero(aes132_entropy, sizeof(aes132_entropy));
    sodium_memzero(aes132_entropy, sizeof(aes132_entropy));
}

void storage_ataes132a_set_mnemonic(uint256_t bin, uint256_t pin_hash) {
    /* Inputs: Mnemonic the user backed up, stretched pin_hash coming directly
     * from secure input.
     *
     * Breaks up the mnemonic and pin information into 3 locations that
     * when xor'd together, results in the user mnemonic.
     *    * ESP32 Secret
     *    * PIN Secret
     *    * UserZone Secret
     * */
    uint8_t res;
    CONFIDENTIAL uint256_t aes132_secret;
    CONFIDENTIAL uint256_t esp_secret;
    CONFIDENTIAL unsigned char child_key[crypto_auth_hmacsha512_BYTES];

    /* One piece of data is purely random, choosing esp_secret to be random */
    rand256(esp_secret);
    xor256(aes132_secret, esp_secret, bin);

    /* Set the pin keys */
    res = aes132_pin_load_keys(pin_hash);
    if( res ) {
        esp_restart();
    }

    /* for each pin key, set the user zone secret */
    res = aes132_pin_load_zones(pin_hash, aes132_secret);
    if( res ) {
        esp_restart();
    }

    uint32_t counter;
    res = aes132_pin_counter(&counter);
    if( res ) {
        esp_restart();
    }
    // Store pin attempt counter
    if( !storage_set_u32(counter, "secret", "last_success") ) {
        esp_restart();
    }
    // Store esp32-side secret
    if( !storage_set_blob(esp_secret, sizeof(esp_secret), "secret", "mnemonic") ) {
        esp_restart();
    }

    sodium_memzero(esp_secret, sizeof(esp_secret));
    sodium_memzero(aes132_secret, sizeof(aes132_secret));
    sodium_memzero(child_key, sizeof(child_key));
}

bool storage_ataes132a_get_mnemonic(uint256_t mnemonic, uint256_t pin_hash) {
    /* returns 256-bit mnemonic from storage 
     * Returns true if mnemonic is returned; false if incorrect pin_hash
     * */
    bool auth = false;
    uint8_t res;
    CONFIDENTIAL uint256_t aes132_secret;
    CONFIDENTIAL uint256_t esp_secret;

    // Attempt Authorization and Get ATAES132A Secret
    res =  aes132_pin_attempt(pin_hash, NULL, aes132_secret);
    if( res ) {
        goto exit;
    }

    // Get ESP32 Secret
    size_t required_size = 32;
    if( !storage_get_blob(esp_secret, &required_size, "secret", "mnemonic") ) {
        esp_restart();
    }

    // Combine Secrets
    xor256(mnemonic, aes132_secret, esp_secret);
    auth = true;
exit:
    sodium_memzero(esp_secret, sizeof(esp_secret));
    sodium_memzero(aes132_secret, sizeof(aes132_secret));
    return auth;
}

uint32_t storage_ataes132a_get_pin_count() {
    /* Gets the current PIN counter. This value is monotomicly increasing. */
    uint32_t counter;
    aes132_pin_counter(&counter);
    return counter;
}

void storage_ataes132a_set_pin_count(uint32_t count) {
    // nothing; pin_count is inherently increased during get_mnemonic attempt
}

uint32_t storage_ataes132a_get_pin_last() {
    /* Gets the pin counter of the last successful pin attempt.
     */
    uint32_t counter;
    if( !storage_get_u32(&counter, "secret", "last_success", 0) ) {
        esp_restart();
    }
    return counter;
}

void storage_ataes132a_set_pin_last(uint32_t count) {
    // Make sure count is always increasing
    uint32_t old_count;
    old_count = storage_ataes132a_get_pin_last();
    if( count > old_count) {
        storage_set_u32(count, "secret", "last_success");
    }
}

bool storage_ataes132a_get_u8(uint8_t *value, char *namespace, char *key,
        uint8_t default_value ) {
    return storage_internal_get_u8(value, namespace, key, default_value);
}

bool storage_ataes132a_set_u8(uint8_t value, char *namespace, char *key) {
    return storage_internal_set_u8(value, namespace, key);
}

bool storage_ataes132a_get_u16(uint16_t *value, char *namespace, char *key,
        uint16_t default_value ) {
    return storage_internal_get_u16(value, namespace, key, default_value);
}

bool storage_ataes132a_set_u16(uint16_t value, char *namespace, char *key) {
    return storage_internal_set_u16(value, namespace, key);
}

bool storage_ataes132a_get_u32(uint32_t *value, char *namespace, char *key,
        uint32_t default_value ) {
    return storage_internal_get_u32(value, namespace, key, default_value);
}

bool storage_ataes132a_set_u32(uint32_t value, char *namespace, char *key) {
    return storage_internal_set_u32(value, namespace, key);
}

bool storage_ataes132a_get_str(char *buf, size_t *required_size,
        char *namespace, char *key, char *default_value) {
    return storage_internal_get_str(buf, required_size, namespace, key,
            default_value);
}

bool storage_ataes132a_set_str(char *str, char *namespace, char *key) {
    return storage_internal_set_str(str, namespace, key);
}

bool storage_ataes132a_get_blob(unsigned char *buf, size_t *required_size,
        char *namespace, char *key) {
    return storage_internal_get_blob(buf, required_size, namespace, key);
}

bool storage_ataes132a_set_blob(unsigned char *buf, size_t len,
        char *namespace, char *key) {
    return storage_internal_set_blob(buf, len, namespace, key);
}

void storage_ataes132a_factory_reset() {
    storage_internal_factory_reset();
    // todo: implement
    return false;
}

bool storage_ataes132a_erase_key(char *namespace, char *key) {
    return storage_internal_erase_key(namespace, key);
}
