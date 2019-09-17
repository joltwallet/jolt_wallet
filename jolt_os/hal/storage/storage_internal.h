/**
 * @file storage_internal.h
 * @brief storing values to the SPI flash NVS partition.
 * @author Brian Pugh
 */

/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_HAL_INTERNAL_STORAGE_H__
#define __JOLT_HAL_INTERNAL_STORAGE_H__

#include "jolttypes.h"
#include "storage.h"

bool storage_internal_startup();

void storage_internal_stretch_init();

void storage_internal_stretch( uint256_t hash, int8_t *progress );

bool storage_internal_exists_mnemonic();

void storage_internal_set_mnemonic( const uint256_t bin, const uint256_t pin_hash );
bool storage_internal_get_mnemonic( uint256_t mnemonic, const uint256_t pin_hash );

uint32_t storage_internal_get_pin_count();
void storage_internal_set_pin_count( uint32_t count );
uint32_t storage_internal_get_pin_last();
void storage_internal_set_pin_last( uint32_t count );

bool storage_internal_get_u8( uint8_t *value, const char *namespace, const char *key, uint8_t default_value );
bool storage_internal_set_u8( uint8_t value, const char *namespace, const char *key );

bool storage_internal_get_u16( uint16_t *value, const char *namespace, const char *key, uint16_t default_value );
bool storage_internal_set_u16( uint16_t value, const char *namespace, const char *key );

bool storage_internal_get_u32( uint32_t *value, const char *namespace, const char *key, uint32_t default_value );
bool storage_internal_set_u32( uint32_t value, const char *namespace, const char *key );

bool storage_internal_get_str( char *buf, size_t *required_size, const char *namespace, const char *key,
                               const char *default_value );
bool storage_internal_set_str( const char *str, const char *namespace, const char *key );

bool storage_internal_get_blob( unsigned char *buf, size_t *required_size, const char *namespace, const char *key );
bool storage_internal_set_blob( const unsigned char *buf, size_t len, const char *namespace, const char *key );

bool storage_internal_erase_key( const char *namespace, const char *key );

void storage_internal_factory_reset();
#endif
