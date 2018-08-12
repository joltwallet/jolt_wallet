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

bool storage_ataes132a_exists_mnemonic() {
}

bool storage_ataes132a_get_mnemonic(uint256_t mnemonic) {
    /* prompt user for pin, returns 256-bit mnemonic from storage 
     * Returns true if mnemonic is returned; false if user cancelled
     * */
    bool res;
exit:
    return res;
}

uint32_t storage_ataes132a_get_pin_count() {
    /* Gets the current PIN counter. This value is monotomicly increasing. */
    uint32_t res;
exit:
    return res;

}

void storage_ataes132a_set_pin_count(uint32_t count) {
}

uint32_t storage_ataes132a_get_pin_last() {
    /* Gets the pin counter of the last successful pin attempt.
     */
    uint32_t res;
exit:
    return res;

}

void storage_ataes132a_set_pin_last(uint32_t count) {
}


bool storage_ataes132a_get(void *value, void *namespace, void *key, void *default_value ) {
    /* Populates [value] from value in storage with [namespace] and [key].
     * If [key] is not found in storage, set [value] to [default_value]
     *
     * Returns true if key was found.
     */
    bool res;
exit:
    return res;

}

bool storage_ataes132a_set(void* value, void *namespace, void *key) {
    /* Stores [value] into [key]. Primarily used for settings.
     *
     * Returns true on success, false on failure.
     * todo: MACRO and function for all types*/
    bool res;
exit:
    return res;
}
