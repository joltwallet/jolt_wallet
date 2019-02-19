/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_VAULT_H__
#define __JOLT_VAULT_H__

#include "nvs.h"
#include "bipmnemonic.h"
#include "jolttypes.h"
#include "lvgl/lvgl.h"


/* Structure to store anything that if modified could perform something
 * malicious
 */
// The private node MUST be set before entering an app
// The private node MUST be cleared upon exiting an app
// Valid Flag can be used as an indicator if user has entered a valid pin
typedef struct vault_t {
    hd_node_t node; // has fields "key" and "chain_code"
    uint32_t purpose;
    uint32_t coin_type;
    char bip32_key[32];
    char passphrase[128];
    bool valid; // If key is valid (hasn't been wiped)
} vault_t;

typedef void (*vault_cb_t)( void *param );

void vault_sem_take();
void vault_sem_give();

bool vault_setup();
void vault_clear();
esp_err_t vault_set(uint32_t purpose, uint32_t coin_type,
        const char *bip32_key, const char *passphrase,
        vault_cb_t failure_cb, vault_cb_t success_cb, void *param);

void vault_refresh(vault_cb_t failure_cb, vault_cb_t success_cb, void *param);

bool vault_kick();
uint32_t vault_get_coin_type();
uint32_t vault_get_purpose();
char *vault_get_bip32_key();
hd_node_t *vault_get_node();

bool vault_is_valid();
bool vault_get_valid();
#endif
