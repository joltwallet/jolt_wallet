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

#include "../globals.h"
#include "../gui/entry.h"
#include "../gui/gui.h"
#include "../helpers.h"
#include "../vault.h"
#include "bipmnemonic.h"
#include "jolttypes.h"
#include "menu8g2.h"
#include "storage_internal.h"

static const char* TAG = "storage_ataes132a";
static const char* TITLE = "Storage Access";

bool storage_ataes132a_exists_mnemonic() {
    return false;
}

void storage_ataes132a_set_mnemonic(uint256_t bin, uint256_t pin_hash) {
    // todo: implement
}

bool storage_ataes132a_get_mnemonic(uint256_t mnemonic, uint256_t pin_hash) {
    /* returns 256-bit mnemonic from storage 
     * Returns true if mnemonic is returned; false if incorrect pin_hash
     * */
    // todo: implement
    return false;
}

uint32_t storage_ataes132a_get_pin_count() {
    /* Gets the current PIN counter. This value is monotomicly increasing. */
    // todo: implement
    return 0;
}

void storage_ataes132a_set_pin_count(uint32_t count) {
    // todo: implement
}

uint32_t storage_ataes132a_get_pin_last() {
    /* Gets the pin counter of the last successful pin attempt.
     */
    // todo: implement
    return 0;
}

void storage_ataes132a_set_pin_last(uint32_t count) {
    // Should probably error check to make sure count is always increasing
    storage_internal_set_pin_last(count);
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

bool storage_ataes132a_get_blob(char *buf, size_t *required_size,
        char *namespace, char *key) {
    return storage_internal_get_blob(buf, required_size, namespace, key);
}

bool storage_ataes132a_set_blob(char *buf, size_t len,
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
