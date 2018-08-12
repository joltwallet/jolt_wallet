/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_HAL_ATAES132A_STORAGE_H__
#define __JOLT_HAL_ATAES132A_STORAGE_H__

bool storage_ataes132a_exists_mnemonic();
bool storage_ataes132a_get_mnemonic(uint256_t mnemonic);
uint32_t storage_ataes132a_get_pin_count();
void storage_ataes132a_set_pin_count(uint32_t count);
uint32_t storage_ataes132a_get_pin_last();
void storage_ataes132a_set_pin_last(uint32_t count);
bool storage_ataes132a_get(void *value, void *namespace, void *key, void *default_value );
bool storage_ataes132a_set(void* value, void *key);

#endif
