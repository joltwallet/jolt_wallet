/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_HAL_ATAES132A_STORAGE_H__
#define __JOLT_HAL_ATAES132A_STORAGE_H__

bool storage_ataes132a_exists_mnemonic();

void storage_ataes132a_set_mnemonic(uint256_t bin, uint256_t pin_hash);
bool storage_ataes132a_get_mnemonic(uint256_t mnemonic, uint256_t pin_hash);

uint32_t storage_ataes132a_get_pin_count();
void storage_ataes132a_set_pin_count(uint32_t count);
uint32_t storage_ataes132a_get_pin_last();
void storage_ataes132a_set_pin_last(uint32_t count);

bool storage_ataes132a_get_u8(uint8_t *value, char *namespace, char *key,
        uint8_t default_value );
bool storage_ataes132a_set_u8(uint8_t value, char *namespace, char *key);

bool storage_ataes132a_get_u16(uint16_t *value, char *namespace, char *key,
        uint16_t default_value );
bool storage_ataes132a_set_u16(uint16_t value, char *namespace, char *key);

bool storage_ataes132a_get_u32(uint32_t *value, char *namespace, char *key,
        uint32_t default_value );
bool storage_ataes132a_set_u32(uint32_t value, char *namespace, char *key);

bool storage_ataes132a_get_str(char *buf, size_t *required_size,
        char *namespace, char *key, char *default_value);
bool storage_ataes132a_set_str(char *str, char *namespace, char *key);

bool storage_ataes132a_get_blob(unsigned char *buf, size_t *required_size,
        char *namespace, char *key);
bool storage_ataes132a_set_blob(unsigned char *buf, size_t len,
        char *namespace, char *key);

void storage_ataes132a_factory_reset();

bool storage_ataes132a_erase_key(char *namespace, char *key);

#endif
