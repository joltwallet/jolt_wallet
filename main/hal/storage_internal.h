/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_HAL_INTERNAL_STORAGE_H__
#define __JOLT_HAL_INTERNAL_STORAGE_H__

#include "jolttypes.h"

bool storage_internal_exists_mnemonic();
void storage_internal_set_mnemonic(uint256_t bin, uint256_t pin_hash);
bool storage_internal_get_mnemonic(uint256_t mnemonic);
uint32_t storage_internal_get_pin_count();
void storage_internal_set_pin_count(uint32_t count);
uint32_t storage_internal_get_pin_last();
void storage_internal_set_pin_last(uint32_t count);
bool storage_internal_get(void *value, void *namespace, void *key, void *default_value );
bool storage_internal_set(void* value, void *key);

#endif
