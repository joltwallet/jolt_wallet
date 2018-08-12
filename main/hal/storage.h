/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_HAL_STORAGE_H__
#define __JOLT_HAL_STORAGE_H__

#include "jolttypes.h"

bool storage_exists_mnemonic();
void storage_set_mnemonic(uint256_t bin);
bool storage_get_mnemonic(char *buf, const uint16_t buf_len);
void storage_set_mnemonic(char *buf, const uint16_t buf_len);
uint32_t storage_get_pin_count();
uint32_t storage_get_pin_last();
bool storage_get_u8(uint8_t *value, char *namespace, char *key, uint8_t *default_value );
bool storage_set_u8(uint8_t value, char *namespace, char *key);
bool storage_get_u32(uint32_t *value, char *namespace, char *key, uint32_t default_value );
bool storage_set_u32(uint32_t value, char *namespace, char *key);
bool storage_get_blob(char *buf, size_t *required_size, char *namespace, char *key, char *default_value );
bool storage_set_blob(char *value, size_t *required_size, char *namespace, char *key);

#endif
