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

bool storage_get_mnemonic(char *buf, const uint16_t buf_len) {
    /* prompt user for pin, returns 256-bit mnemonic from storage 
     * Returns true if mnemonic is returned; false if user cancelled
     * */
    bool res;
    CONFIDENTIAL uint256_t bin;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_get_mnemonic(bin);
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_get_mnemonic(bin);
#endif
    if( res ){
        jolt_err_t err = bm_bin_to_mnemonic(buf, buf_len, bin, 256);
        if( E_SUCCESS != err ){
            esp_restart();
        }
    }
    sodium_memzero(bin, sizeof(bin));
    return res;
}

uint32_t storage_get_pin_count() {
    /* Gets the current PIN counter. This value is monotomicly increasing. */
    uint32_t res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_get_pin_count();
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_get_pin_count();
#endif
    return res;
}

void storage_set_pin_count(uint32_t count) {
    /* Sets the current PIN counter. This value is monotomicly increasing. */
#if CONFIG_JOLT_STORE_INTERNAL
    storage_internal_set_pin_count(count);
#elif CONFIG_JOLT_STORE_ATAES132A
    storage_ataes132a_set_pin_count(count);
#endif
}

uint32_t storage_get_pin_last() {
    /* Gets the pin counter of the last successful pin attempt.
     */
    uint32_t res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_get_pin_last();
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_get_pin_last();
#endif
    return res;
}

void storage_set_pin_last(uint32_t count) {
    /* Gets the pin counter of the last successful pin attempt.
     */
#if CONFIG_JOLT_STORE_INTERNAL
    storage_internal_set_pin_last(count);
#elif CONFIG_JOLT_STORE_ATAES132A
    storage_ataes132a_set_pin_last(count);
#endif
}

bool storage_get(void *value, void *namespace, void *key, void *default_value ) {
    /* Populates [value] from value in storage with [namespace] and [key].
     * If [key] is not found in storage, set [value] to [default_value]
     *
     * Returns true if key was found.
     */
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_get(value, namespace, key, default_value);
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_get(value, namespace, key, default_value);
#endif
    return res;

}

bool storage_set(void* value, void *key) {
    /* Stores [value] into [key]. Primarily used for settings.
     *
     * Returns true on success, false on failure.
     * todo: MACRO and function for all types*/
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_set(value, key); 
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_set(value, key); 
#endif
    return res;
}
