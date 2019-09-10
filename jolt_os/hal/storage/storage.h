/**
 * @file storage.h
 * @brief Unified non-volatile storage interface
 * @author Brian Pugh
 */

/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_HAL_STORAGE_H__
#define __JOLT_HAL_STORAGE_H__

#include "jolttypes.h"

/**
 * @brief Initialize any components necessary to utilize NVS.
 * @return True on success, false otherwise.
 */
bool storage_startup();

/**
 * @brief Setup a secret used for PIN stretching
 */
void storage_stretch_init();

/**
 * @brief Stretch a hash in-place. Used to slow down PIN attempts.
 *
 * This is part of storage because we use the ATAES132A's encryption as a form
 * of stretching.
 *
 * Provides some security by slowing down PIN attempts if the PIN attempt counter has somehow been bypassed, but the secrets for stretching haven't been leaked.
 *
 * @param[in,out] hash Input hash to be stretched. Output of stretching is written back here.
 * @param[out] progress Updated [0~100] to reflect stretch progress
 */
void storage_stretch( uint256_t hash, int8_t *progress );

/**
 * @brief Checks to see if the data used to reassemble the 256-bit mnemonic exists in storage.
 * @return True if data exists, false otherwise.
 */
bool storage_exists_mnemonic();

/**
 * @brief Stores the 256-bit binary representing the mnemonic.
 *
 * Before saving, it internally uses the stretched pin_hash for some form on encryption, making 
 * the stretched pin_hash required for decryption. 
 *
 * @param[in] bin Vanilla 256-bit mnemonic in binary form.
 * @param[in] pin_hash Stretched hash derived from PIN. Must previously call storage_stretch.
 * */
void storage_set_mnemonic(const uint256_t bin, const uint256_t pin_hash);

/**
 * @brief Assembles the complete vanilla 256-binary mnemonic from storage.
 *
 * Populates the 256-bit binary mnemonic field; Uses the provided stretched
 * pin_hash as a part of decryption. 
 *
 * Internally, this function increments the PIN attempt counter.
 *
 * @param[out] bin Vanilla 256-bit mnemonic in binary form.
 *
 * @return True on successful decryption, false otherwise
 */
bool storage_get_mnemonic(uint256_t bin, const uint256_t pin_hash);

/**
 * @brief Get the number of monotonic increasing PIN attempts performed
 *
 * If using internal storage, this is the number of PIN attempts since last factory reset.
 * If using ATAES132A, this is the number of PIN attempts over the lifetime of the device.
 *
 * @return Number of PIN attempts recorded
 */
uint32_t storage_get_pin_count();

/**
 * @brief Set the PIN attempt count number
 *
 * Only valid when using internal storage. ATAES132A counters cannot be decremented.
 *
 * @param[in] New number of pin attempts.
 */
void storage_set_pin_count(uint32_t count);

/**
 * @brief Get the monotonically increasing PIN counter the last time the correct PIN was entered
 *
 * Since this number is stored in flash, it is susceptible to a flash replay attack.
 * To mitigate this, the counter is always increasing so that more PIN attempts cannot be gained by
 * restoring a previously known encrypted flash image while using the ATAES132a.
 *
 * @return Number of PIN attempts recorded at the last successful PIN entry. 
 */
uint32_t storage_get_pin_last();

/** 
 * @brief Set the monotonically increasing last success PIN entry count value in storage.
 * @param[in] count Entry counter value to store
 */
void storage_set_pin_last(uint32_t count);

/**
 * @brief Get a previously stored uint8_t value at key
 * @param[out] value Buffer to store retrieved value.
 * @param[in] namespace Namespace to lookup key in.
 * @param[in] key Key to lookup in within Namespace in storage.
 * @param[in] default_value Copied into value if key isn't found in storage.
 * @return True if value was found in storage, false if the default value was copied into value.
 */
bool storage_get_u8(uint8_t *value, const char *namespace, const char *key, uint8_t default_value );

/**
 * @brief Store a uint8_t value into non-volatile storage
 * @param[in] value Value to store.
 * @param[in] namespace Namespace to lookup key in.
 * @param[in] key Key to lookup in within Namespace in storage.
 * @return True on a successful store, false otherwise.
 */
bool storage_set_u8(uint8_t value, const char *namespace, const char *key);

/**
 * @brief Get a previously stored uint16_t value at key
 * @param[out] value Buffer to store retrieved value.
 * @param[in] namespace Namespace to lookup key in.
 * @param[in] key Key to lookup in within Namespace in storage.
 * @param[in] default_value Copied into value if key isn't found in storage.
 * @return True if value was found in storage, false if the default value was copied into value.
 */
bool storage_get_u16(uint16_t *value, const char *namespace, const char *key, uint16_t default_value );

/**
 * @brief Store a uint16_t value into non-volatile storage
 * @param[in] value Value to store.
 * @param[in] namespace Namespace to lookup key in.
 * @param[in] key Key to lookup in within Namespace in storage.
 * @return True on a successful store, false otherwise.
 */

bool storage_set_u16(uint16_t value, const char *namespace, const char *key);

/**
 * @brief Get a previously stored uint32_t value at key
 * @param[out] value Buffer to store retrieved value.
 * @param[in] namespace Namespace to lookup key in.
 * @param[in] key Key to lookup in within Namespace in storage.
 * @param[in] default_value Copied into value if key isn't found in storage.
 * @return True if value was found in storage, false if the default value was copied into value.
 */
bool storage_get_u32(uint32_t *value, const char *namespace, const char *key, uint32_t default_value );

/**
 * @brief Store a uint32_t value into non-volatile storage
 * @param[in] value Value to store.
 * @param[in] namespace Namespace to lookup key in.
 * @param[in] key Key to lookup in within Namespace in storage.
 * @return True on a successful store, false otherwise.
 */

bool storage_set_u32(uint32_t value, const char *namespace, const char *key);

/**
 * @brief Get a previously stored binary blob at key
 * @param[out] buf Buffer to store retrieved binary blob.
 * @param[out] required_size If buf is NULL, required_size will be updated with the space required to load the stored binary blob.
 * @param[in] namespace Namespace to lookup key in.
 * @param[in] key Key to lookup in within Namespace in storage.
 * @return True if value was found in storage, false if the default value was not found.
 */
bool storage_get_blob(unsigned char *buf, size_t *required_size, const char *namespace, const char *key);

/**
 * @brief Store a binary blob into non-volatile storage
 * @param[in] value Value to store.
 * @param[in] len Length of binary blob.
 * @param[in] namespace Namespace to lookup key in.
 * @param[in] key Key to lookup in within Namespace in storage.
 * @return True on a successful store, false otherwise.
 */
bool storage_set_blob(const unsigned char *buf, size_t len, const char *namespace, const char *key);

/**
 * @brief Get a previously stored NULL-terminated string at key
 * @param[out] buf Buffer to store retrieved string.
 * @param[out] required_size If buf is NULL, required_size will be updated with the space required to load the stored binary blob.
 * @param[in] namespace Namespace to lookup key in.
 * @param[in] key Key to lookup in within Namespace in storage.
 * @param[in] default_value Copied into buf if key isn't found in storage.
 * @return True if value was found in storage, false if the default value was not found.
 */
bool storage_get_str(char *buf, size_t *required_size, const char *namespace, const char *key, const char *default_value);

/**
 * @brief Store a NULL-terminated string into non-volatile storage
 * @param[in] str NULL-terminated string to store.
 * @param[in] namespace Namespace to lookup key in.
 * @param[in] key Key to lookup in within Namespace in storage.
 * @return True on a successful store, false otherwise.
 */
bool storage_set_str(const char *str, const char *namespace, const char *key);

/**
 * @brief Erase a key from storage
 * @param[in] namespace Namespace to lookup key in.
 * @param[in] key Key to lookup in within Namespace in storage.
 * @return True if key was erase; false if key wasn't found.
 */
bool storage_erase_key(const char *namespace, const char *key);

/**
 * @brief Clear all keys and filesystems.
 * @param reset Reset after clearing data.
 */
void storage_factory_reset( bool reset );

#endif
