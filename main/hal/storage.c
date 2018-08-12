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

#if CONFIG_JOLT_STORE_INTERNAL
#include "storage_internal.h"
#elif CONFIG_JOLT_STORE_ATAES132A
#include "storage_ataes132a.h"
#endif

static const char* TAG = "storage_hal";
static const char* TITLE = "Storage Access";

bool storage_exists_mnemonic() {
    /* Checks if a mnemonic has been previously setup on the device.
     * Returns true if mnemonic has been setup; false otherwise*/
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_exists_mnemonic();
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_exists_mnemonic();
#endif
   return res;
}

void storage_set_mnemonic(uint256_t bin, uint256_t pin_hash) {
    /* store binary mnemonic */
#if CONFIG_JOLT_STORE_INTERNAL
    storage_internal_set_mnemonic(bin, pin_hash);
#elif CONFIG_JOLT_STORE_ATAES132A
    storage_ataes132a_set_mnemonic(bin, pin_hash);
#endif
    return;
}

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

bool storage_get_u8(uint8_t *value, char *namespace, char *key, uint8_t *default_value ) {
    /* Populates [value] from value in storage with [namespace] and [key].
     * If [key] is not found in storage, set [value] to [default_value]
     *
     * Returns true if key was found.
     */
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_get_u8(value, namespace, key, default_value);
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_get_u8(value, namespace, key, default_value);
#endif
    return res;
}

bool storage_set_u8(uint8_t value, char *namespace, char *key) {
    /* Stores [value] into [key]. Primarily used for settings.
     *
     * Returns true on success, false on failure.
     * todo: MACRO and function for all types*/
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_set_u8(value, namespace, key); 
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_set_u8(value, namespace, key); 
#endif
    return res;
}

bool storage_get_u32(uint32_t *value, char *namespace, char *key, uint32_t default_value ) {
    /* Populates [value] from value in storage with [namespace] and [key].
     * If [key] is not found in storage, set [value] to [default_value]
     *
     * Returns true if key was found.
     */
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_get_u32(value, namespace, key, default_value);
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_get_u32(value, namespace, key, default_value);
#endif
    return res;

}

bool storage_set_u32(uint32_t value, char *namespace, char *key) {
    /* Stores [value] into [key]. Primarily used for settings.
     *
     * Returns true on success, false on failure.
     * todo: MACRO and function for all types*/
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_set_u32(value, namespace, key); 
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_set_u32(value, namespace, key); 
#endif
    return res;
}
bool storage_get_blob(char *buf, size_t *required_size, char *namespace, char *key, char *default_value ) {
    /* Populates [value] from value in storage with [namespace] and [key].
     * If [key] is not found in storage, set [value] to [default_value]
     *
     * Returns true if key was found.
     */
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_get_blob(buf, required_size, namespace, key, default_value);
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_get_blob(buf, required_size, namespace, key, default_value);
#endif
    return res;
}

bool storage_set_blob(char *value, size_t *required_size, char *namespace, char *key) {
    /* Stores [value] into [key]. Primarily used for settings.
     *
     * Returns true on success, false on failure.
     * todo: MACRO and function for all types*/
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_set_blob(value, required_size, namespace, key); 
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_set_blob(value, required_size, namespace, key); 
#endif
    return res;
}
