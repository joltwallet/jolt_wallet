/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_VAULT_H__
#define __JOLT_VAULT_H__

#include "nvs.h"
#include "nano_lib.h"
#include "bipmnemonic.h"
#include "jolttypes.h"

void setup_private_node();

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
    bool valid; // If key is valid (hasn't been wiped)
} vault_t;

void setup_vault();
jolt_err_t init_nvm_namespace(nvs_handle *nvs_h, const char *namespace);

#endif
