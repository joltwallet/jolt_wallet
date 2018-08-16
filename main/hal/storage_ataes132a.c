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
#include "../helpers.h"
#include "../globals.h"
#include "../vault.h"
#include "../gui/gui.h"
#include "../gui/entry.h"
#include "../gui/statusbar.h"
#include "../gui/loading.h"
#include "storage_internal.h"

static const char* TAG = "ataes132a";
static const char* TITLE = "Storage Access";

bool storage_ataes132a_exists_mnemonic() {
    return false;
}

void storage_ataes132a_set_mnemonic(uint256_t bin, uint256_t pin_hash) {
}

bool storage_ataes132a_get_mnemonic(uint256_t mnemonic, uint256_t pin_hash) {
    /* returns 256-bit mnemonic from storage 
     * Returns true if mnemonic is returned; false if incorrect pin_hash
     * */
    return false;
}

uint32_t storage_ataes132a_get_pin_count() {
    /* Gets the current PIN counter. This value is monotomicly increasing. */
    return 0;
}

void storage_ataes132a_set_pin_count(uint32_t count) {
}

uint32_t storage_ataes132a_get_pin_last() {
    /* Gets the pin counter of the last successful pin attempt.
     */
    return 0;
}

void storage_ataes132a_set_pin_last(uint32_t count) {
}

bool storage_ataes132a_get_u8(uint8_t *value, char *namespace, char *key,
        uint8_t default_value ) {
    return false;
}

bool storage_ataes132a_set_u8(uint8_t value, char *namespace, char *key) {
    return false;
}

bool storage_ataes132a_get_u16(uint16_t *value, char *namespace, char *key,
        uint16_t default_value ) {
    return false;
}

bool storage_ataes132a_set_u16(uint16_t value, char *namespace, char *key) {
    return false;
}

bool storage_ataes132a_get_u32(uint32_t *value, char *namespace, char *key,
        uint32_t default_value ) {
    return false;
}

bool storage_ataes132a_set_u32(uint32_t value, char *namespace, char *key) {
    return false;
}

bool storage_ataes132a_get_str(char *buf, size_t *required_size,
        char *namespace, char *key, char *default_value) {
    return false;
}

bool storage_ataes132a_set_str(char *str, char *namespace, char *key) {
    return false;
}

bool storage_ataes132a_get_blob(char *buf, size_t *required_size,
        char *namespace, char *key) {
    return false;
}

bool storage_ataes132a_set_blob(char *buf, size_t len,
        char *namespace, char *key) {
    return false;
}

void storage_ataes132a_factory_reset() {
    return false;
}

bool storage_ataes132a_erase_key(char *namespace, char *key) {
    return false;
}
