/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_HAL_STORAGE_H__
#define __JOLT_HAL_STORAGE_H__

#include "jolttypes.h"
#include "jolt_gui/jolt_gui_stretch.h"

bool storage_startup();

/* Stretches and stores the value in jolt_gui_store.derivation.pin.
 * Creates and updates the loading screen as stretching progresses.*/ 
void storage_stretch_task(jolt_derivation_t *stretch);

bool storage_exists_mnemonic();

void storage_set_mnemonic(const uint256_t bin, const uint256_t pin_hash);
bool storage_get_mnemonic(uint256_t bin, const uint256_t pin_hash);

uint32_t storage_get_pin_count();
void storage_set_pin_count(uint32_t count);
uint32_t storage_get_pin_last();
void storage_set_pin_last(uint32_t count);

bool storage_get_u8(uint8_t *value, char *namespace, char *key, uint8_t default_value );
bool storage_set_u8(uint8_t value, char *namespace, char *key);

bool storage_get_u16(uint16_t *value, char *namespace, char *key, uint16_t default_value );
bool storage_set_u16(uint16_t value, char *namespace, char *key);

bool storage_get_u32(uint32_t *value, char *namespace, char *key, uint32_t default_value );
bool storage_set_u32(uint32_t value, char *namespace, char *key);

bool storage_get_blob(unsigned char *buf, size_t *required_size, char *namespace, char *key);
bool storage_set_blob(unsigned char *buf, size_t len, char *namespace, char *key);

bool storage_get_str(char *buf, size_t *required_size, char *namespace, char *key, char *default_value);
bool storage_set_str(char *str, char *namespace, char *key);

bool storage_erase_key(char *namespace, char *key);

void storage_factory_reset();

#endif
