/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_HAL_STORAGE_H__
#define __JOLT_HAL_STORAGE_H__

bool storage_get_mnemonic(char *buf, const uint16_t buf_len);
uint32_t storage_get_pin_count();
uint32_t storage_get_pin_last();
bool storage_get(void *value, void *namespace, void *key, void *default_value );
bool storage_set(void* value, void *key);

#endif
