/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

//#define LOG_LOCAL_LEVEL 4

#include "storage_ataes132a.h"
#include <esp_system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aes132_cmd.h"
#include "aes132_comm_marshaling.h"
#include "aes132_jolt.h"
#include "bipmnemonic.h"
#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_helpers.h"
#include "jolttypes.h"
#include "sodium.h"
#include "storage.h"
#include "storage_internal.h"
#include "vault.h"

#if CONFIG_JOLT_STORE_ATAES132A

static const char *TAG = "storage_ataes132a";

bool storage_ataes132a_startup()
{
    if( !( storage_internal_startup() && 0 == aes132_jolt_setup() ) ) {
        // something bad happened; but we should never get here because
        // aes132_jolt_setup() should restart esp32 at the slightest hint
        // of error.
        return false;
    }
    return true;
}

bool storage_ataes132a_exists_mnemonic()
{
    /* Returns true if mnemonic exists, false otherwise */
    bool res;
    res = storage_internal_exists_mnemonic();
    return res;
}

/**
 * Generate a new stretch key on the ATAES132a that never leaves it.
 * Also generate a stretching secret to prevent some physical attacks from
 * directly deriving your PIN.
 */
void storage_ataes132a_stretch_init()
{
    CONFIDENTIAL uint8_t stretch_secret[32];
    jolt_get_random( stretch_secret, sizeof( stretch_secret ) );
    storage_set_blob( stretch_secret, sizeof( stretch_secret ), "secret", "stretch" );
    sodium_memzero( stretch_secret, sizeof( stretch_secret ) );
    if( 0 != aes132_create_stretch_key() ) esp_restart();
}

void storage_ataes132a_stretch( uint256_t hash, int8_t *progress )
{
    CONFIDENTIAL uint8_t stretch_secret[32];
    CONFIDENTIAL unsigned char result[crypto_auth_hmacsha512_BYTES];
    size_t required_size = sizeof( stretch_secret );

    /* Retrieve stretching secret */
    if( !storage_get_blob( stretch_secret, &required_size, "secret", "stretch" ) ) {
        // Failed to get stretch_secret
        storage_factory_reset( true );
    }

    assert( crypto_auth_hmacsha512_KEYBYTES == 32 );
    crypto_auth_hmacsha512( result, hash, 32, stretch_secret );

    /* Perform Legacy commands on ATAES132a */
    // Only the first 128-bits are stretched since the authentication keys on
    // the ataes132a are 128-bit.
    if( 0 != aes132_stretch( result, 16, CONFIG_JOLT_AES132_STRETCH_ITER, progress ) ) esp_restart();

    memcpy( hash, result, 32 );

    sodium_memzero( stretch_secret, sizeof( stretch_secret ) );
    sodium_memzero( result, sizeof( result ) );
}

/**
 * @brief XOR 2 256-bit arrays.
 *
 * Output may be an input.
 *
 * @param[out] out xor of `x` and `y`
 * @param[in] x
 * @param[in] y
 */
static void xor256( uint8_t *out, const uint8_t *x, const uint8_t *y )
{
    for( uint8_t i = 0; i < 32; i++ ) { out[i] = x[i] ^ y[i]; }
}

void storage_ataes132a_set_mnemonic( const uint256_t bin, const uint256_t pin_hash )
{
    /* Inputs: Mnemonic the user backed up, stretched pin_hash coming directly
     * from secure input.
     *
     * Breaks up the mnemonic and pin information into 3 locations that
     * when xor'd together, results in the user mnemonic.
     *    * ESP32 Secret
     *    * PIN Secret
     *    * UserZone Secret
     * */
    CONFIDENTIAL uint256_t aes132_secret;
    CONFIDENTIAL uint256_t esp_secret;

    /* One piece of data is purely random, choosing esp_secret to be random */
    jolt_get_random( esp_secret, sizeof( esp_secret ) );
    xor256( aes132_secret, esp_secret, bin );

    /* Set the pin keys */
    if( aes132_pin_load_keys( pin_hash ) ) esp_restart();

    ESP_LOGD( TAG,
              "Storing ATAES132a Secret: "
              "%02X %02X %02X %02X %02X %02X %02X %02X "
              "%02X %02X %02X %02X %02X %02X %02X %02X "
              "%02X %02X %02X %02X %02X %02X %02X %02X "
              "%02X %02X %02X %02X %02X %02X %02X %02X ",
              aes132_secret[0], aes132_secret[1], aes132_secret[2], aes132_secret[3], aes132_secret[4],
              aes132_secret[5], aes132_secret[6], aes132_secret[7], aes132_secret[8], aes132_secret[9],
              aes132_secret[10], aes132_secret[11], aes132_secret[12], aes132_secret[13], aes132_secret[14],
              aes132_secret[15], aes132_secret[16], aes132_secret[17], aes132_secret[18], aes132_secret[19],
              aes132_secret[20], aes132_secret[21], aes132_secret[22], aes132_secret[23], aes132_secret[24],
              aes132_secret[25], aes132_secret[26], aes132_secret[27], aes132_secret[28], aes132_secret[29],
              aes132_secret[30], aes132_secret[31] );

    /* for each pin key, set the user zone secret */
    if( aes132_pin_load_zones( pin_hash, aes132_secret ) ) esp_restart();

    uint32_t counter;
    if( aes132_pin_counter( &counter ) ) {
        ESP_LOGE( TAG, "could not retrieve \"pin_counter\"" );
        esp_restart();
    }
    // Store pin attempt counter
    if( !storage_set_u32( counter, "secret", "last_success" ) ) {
        ESP_LOGE( TAG, "could not set \"last_success\"" );
        esp_restart();
    }
    // Store esp32-side secret
    if( !storage_set_blob( esp_secret, sizeof( esp_secret ), "secret", "mnemonic" ) ) {
        ESP_LOGE( TAG, "could not set \"mnemonic\"" );
        esp_restart();
    }

    sodium_memzero( esp_secret, sizeof( esp_secret ) );
    sodium_memzero( aes132_secret, sizeof( aes132_secret ) );
}

bool storage_ataes132a_get_mnemonic( uint256_t mnemonic, const uint256_t pin_hash )
{
    /* returns 256-bit mnemonic from storage
     * Returns true if mnemonic is returned; false if incorrect pin_hash
     * */
    bool auth = false;
    CONFIDENTIAL uint256_t aes132_secret;
    CONFIDENTIAL uint256_t esp_secret;
    uint32_t counter = 0;

    // Attempt Authorization and Get ATAES132A Secret
    if( aes132_pin_attempt( pin_hash, &counter, aes132_secret ) ) goto exit;

    // Get ESP32 Secret
    size_t required_size = 32;
    if( !storage_get_blob( esp_secret, &required_size, "secret", "mnemonic" ) ) {
        ESP_LOGE( TAG, "could not retrieve \"mnemonic\"" );
        esp_restart();
    }

    // Combine Secrets
    xor256( mnemonic, aes132_secret, esp_secret );
    auth = true;

    /* Add one since counter reflects the value prior to this successful pin attempt */
    storage_ataes132a_set_pin_last( counter + 1 );

exit:
    sodium_memzero( esp_secret, sizeof( esp_secret ) );
    sodium_memzero( aes132_secret, sizeof( aes132_secret ) );
    return auth;
}

uint32_t storage_ataes132a_get_pin_count()
{
    /* Gets the current PIN counter. This value is monotomicly increasing. */
    uint32_t counter;
    aes132_pin_counter( &counter );
    return counter;
}

void storage_ataes132a_set_pin_count( uint32_t count )
{
    /* nothing; pin_count is inherently increased during get_mnemonic attempt */
}

uint32_t storage_ataes132a_get_pin_last()
{
    /* Gets the pin counter of the last successful pin attempt.
     */
    uint32_t counter;
    if( !storage_get_u32( &counter, "secret", "last_success", 0 ) ) {
        ESP_LOGE( TAG, "could not retrieve \"last_success\"" );
        esp_restart();
    }
    return counter;
}

void storage_ataes132a_set_pin_last( uint32_t count )
{
    // Make sure count is always increasing
    uint32_t old_count;
    old_count = storage_ataes132a_get_pin_last();
    if( count > old_count ) { storage_set_u32( count, "secret", "last_success" ); }
}

bool storage_ataes132a_get_u8( uint8_t *value, const char *namespace, const char *key, uint8_t default_value )
{
    return storage_internal_get_u8( value, namespace, key, default_value );
}

bool storage_ataes132a_set_u8( uint8_t value, const char *namespace, const char *key )
{
    return storage_internal_set_u8( value, namespace, key );
}

bool storage_ataes132a_get_u16( uint16_t *value, const char *namespace, const char *key, uint16_t default_value )
{
    return storage_internal_get_u16( value, namespace, key, default_value );
}

bool storage_ataes132a_set_u16( uint16_t value, const char *namespace, const char *key )
{
    return storage_internal_set_u16( value, namespace, key );
}

bool storage_ataes132a_get_u32( uint32_t *value, const char *namespace, const char *key, uint32_t default_value )
{
    return storage_internal_get_u32( value, namespace, key, default_value );
}

bool storage_ataes132a_set_u32( uint32_t value, const char *namespace, const char *key )
{
    return storage_internal_set_u32( value, namespace, key );
}

bool storage_ataes132a_get_str( char *buf, size_t *required_size, const char *namespace, const char *key,
                                const char *default_value )
{
    return storage_internal_get_str( buf, required_size, namespace, key, default_value );
}

bool storage_ataes132a_set_str( const char *str, const char *namespace, const char *key )
{
    return storage_internal_set_str( str, namespace, key );
}

bool storage_ataes132a_get_blob( unsigned char *buf, size_t *required_size, const char *namespace, const char *key )
{
    return storage_internal_get_blob( buf, required_size, namespace, key );
}

bool storage_ataes132a_set_blob( const unsigned char *buf, size_t len, const char *namespace, const char *key )
{
    return storage_internal_set_blob( buf, len, namespace, key );
}

void storage_ataes132a_factory_reset()
{
    aes132_create_stretch_key();  // Effectively destroys the mnemonic.
    storage_internal_factory_reset();
}

bool storage_ataes132a_erase_key( const char *namespace, const char *key )
{
    return storage_internal_erase_key( namespace, key );
}
#endif
