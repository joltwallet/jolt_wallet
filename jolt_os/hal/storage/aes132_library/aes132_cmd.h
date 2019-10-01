/**
 * @file aes132_cmd.h
 * @author Brian Pugh
 * @brief Commands to easily interact with the AES132 chip.
 */

#ifndef __AES132_CMD_H__
#define __AES132_CMD_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "aes132_mac.h"
#include "jolttypes.h"

#define AES132_OK 0

/**
 * @brief Authenticate a key; unlocks corresponding UserZone
 *
 * Used to check a PIN attempt. Inherently increases the attempt counter.
 *
 * @param[in] key 128-bit key
 * @param[in] key_id Key index to authenticate
 * @param[in] nonce
 * @return 0 on success
 */
uint8_t aes132_auth( const uint8_t *key, uint16_t key_id, struct aes132h_nonce_s *nonce );

/**
 * @brief BlockRead Command reads 1~32 bytes of plaintext from userzone or configuration memory.
 *
 * Standard eeprom read commands can also read userzone data if authentication nor encryption is required.
 * Standard eeprom read commands *cannot* read configuration memory.
 *
 * Requested data *cannot* cross page boundaries.
 *
 * Configuration memory can *only* be read via the BlockRead command.
 *
 * @param[in] data
 * @param[in] address
 * @param[in] count
 * @return 0 on success
 */
uint8_t aes132_blockread( uint8_t *data, const uint16_t address, const uint8_t count );

/**
 * @brief Checks if device configuration is locked.
 * @param[out] status Returns `true` if device configuration is locked.
 * @return 0 on success
 */
uint8_t aes132_check_configlock( bool *status );

/**
 * @brief Get the counter value of a key.
 *
 * @param[in] mac_key 16-byte key used to verify mac
 * @param[out] count Returned count value
 * @param[in] counter_id Counter index to query.
 * @param[in] nonce
 * @return 0 on success
 */
uint8_t aes132_counter( const uint8_t *mac_key, uint32_t *count, uint8_t counter_id, struct aes132h_nonce_s *nonce );

/**
 * @brief Encrypts upto 32 bytes of data
 *
 * @param[in] in Cleartext data buffer to encrypt.
 * @param[in] len Length of `in` buffer. Maximum 32 bytes.
 * @param[in] key_id
 * @param[out] out_data Produced Cipher text. 16 bytes (<= 16 bytes in) or 32 bytes (<= 3 bytes in) long.
 * @param[out] out_mac
 * @param[in] nonce
 * @return 0 on success
 */
uint8_t aes132_encrypt( const uint8_t *in, uint8_t len, uint8_t key_id, uint8_t *out_data, uint8_t *out_mac,
                        struct aes132h_nonce_s *nonce );

/**
 * @brief Creates a random key on device.
 *
 * Used to securely create the PIN stretch key since we don't actually care exactly what it is.
 *
 * @param[in] key_id Index to use
 * @param[in] nonce
 * @return 0 on success
 */
uint8_t aes132_key_create( uint8_t key_id, struct aes132h_nonce_s *nonce );

/**
 * @brief
 *    We use this to load in the PIN authorization key
 *    Encrypts child_key with parent_key, and sends it over
 * @param[in] parent_key 16-byte key to encrypt child key prior to sending it over the I2C bus. Must match the key_id's
 * parent key.
 * @param[in] child_key 16-byte key to load into key_id keystore
 * @param[in] key_id Key index to use.
 * @param[in] nonce
 * @return 0 on success
 */
uint8_t aes132_key_load( const uint128_t parent_key, uint128_t child_key, const uint8_t key_id,
                         struct aes132h_nonce_s *nonce );

/**
 * @brief Performs ECB encryption on data.
 *
 * This just runs the data through the AES engine.
 *
 * Do NOT use for generic encryption
 *
 * Jolt primarily uses this function for PIN stretching.
 *
 * @param[in] key_id Key index to use.
 * @param[in, out] data pointer to 16 bytes of data to run through the AES engine. Results are stored in same buffer.
 */
uint8_t aes132_legacy( const uint8_t key_id, uint8_t *data );

/**
 * @brief Permamently locks smallzone, config memory, key memory, and makes the master UserZone read-only
 *
 * WARNING this function has permament physical impacts.
 *
 * @return 0 on success;
 */
uint8_t aes132_lock_device();

/**
 * @brief Get the remote MAC count.
 * @param[out] device mac count.
 * @return 0 on success
 */
uint8_t aes132_mac_count( uint8_t *count );

/**
 * @brief Increments the local MAC counter. Will issue a new random Nonce when MAC counter reaches maximum value.
 * @param[in] nonce Command configuration
 * @return Updated local mac count
 */
uint8_t aes132_mac_incr( struct aes132h_nonce_s *nonce );

/**
 * @brief send NONCE command to device.
 *
 * Updates the global nonce variable.
 *
 * @param[in] nonce Command configuration
 * @param[in] in_seed Used as the in_seed if provided. Either NULL or a pointer to a 12-byte buffer.
 */
uint8_t aes132_nonce( struct aes132h_nonce_s *nonce, const uint8_t *in_seed );

/**
 * @brief Generate n_bytes of cryptographically strong random data.
 * @param[out] out Buffer to store random data. XORs with the currently available
 *             buffer. This is useful if combining RNG sources, and has no
 *             impact if not.
 * @param[in] n_bytes Number of random bytes to generate.
 * @return 0 on success
 */
uint8_t aes132_rand( uint8_t *out, const size_t n_bytes );

#endif
