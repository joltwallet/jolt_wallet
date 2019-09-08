#ifndef __AES132_JOLT_H__
#define __AES132_JOLT_H__

/**
 * Creates a new key in the stretch slot. Will cause loss of mnemonic!!!
 * @return 0 on success.
 */
uint8_t aes132_create_stretch_key();

/**
 * Should be first thing called after setting up i2c.
 *
 * The primary goal of this function is to get the ATAES132a into a useable state 
 * for Jolt.
 *
 * This includes:
 *     1. Generating a Master Key using the ESP32 onboard entropy. This key is 
 *        used to change keys to storage zones. The security of the Master Key 
 *        isn't critical to the security of the mnemonic; it is assumed that an
 *        attacker has access to the master key. Master Key isn't fixed just 
 *        to be a little more difficult.
 *     2. Store the Master Key both locally, and back it up to the ATAES132a chip.
 *     3. Configure and Lock the ATAES132a. This activates the cryptographically
 *        secure RNG generator.
 *
 * At the end of this function, ATAES132a is ready to go and Master Key is in RAM.
 * @return 0 on success.
 */
uint8_t aes132_jolt_setup();

/**
 * Only attempts key and returns the cumulative counter.
 *
 * @param[in] optional. 256-bit stretched key. A child key is derived from this 
 *            based on the Zone authentication is attempted onn.
 *            If NULL, no attempt will be made.
 * @param[out] counter optional. The cumulative attempt counter prior to this attempt.
 * @param[out] secret Upon successful authentication, this is the resulting 
 *             256-bit secret 
 *             (XOR of `child key` and data in the UserZone).
 * @return 0 on success.
 */
uint8_t aes132_pin_attempt(const uint8_t *key, uint32_t *counter, uint8_t *secret);

/**
 * Derives and sets childkeys for each zone.
 *
 * @param[in] key 256-bit stretched key
 * @return 0 on success.
 */
uint8_t aes132_pin_load_keys(const uint8_t *key);

/**
 * @brief store derived secret into each UserZone.
 *
 * Run `aes132_pin_load_keys` prior to running this function with the same key.
 *
 * Stores the value (child key ^ secret) into each user zone.  
 * This is so that changing the child key destroys the XOR secret.
 *
 * @param[in] 256-bit stretched key.
 * @param[in] Some secret to store derived copies of.
 * @return 0 on success.
 */
uint8_t aes132_pin_load_zones(const uint8_t *key, const uint8_t *secret);

/**
 * Populates counter value from ataes132a 
 * @param[out] cumulative PIN attempt counter.
 * @return 0 on success.
 */
uint8_t aes132_pin_counter(uint32_t *counter);

/**
 * Calls ECB using stretch-key on each 128bit chunk of input `n_iter` times
 * sequentially.
 *
 * @param[in,out] data data to be stretched inplace.
 * @param[in] data_len Length of data in bytes.
 * @param[in] n_iter Number of times to consecutively run the ECB Legacy command.
 * @param[out] progress, optional pointer to populate with percentage from 0 to 100.
 * @return 0 on success.
 */
uint8_t aes132_stretch(uint8_t *data, const uint8_t data_len, uint32_t n_iter, int8_t *progress);

#endif
