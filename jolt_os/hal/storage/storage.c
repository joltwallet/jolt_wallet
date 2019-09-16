/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "storage.h"
#include <esp_system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bipmnemonic.h"
#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_helpers.h"
#include "jolttypes.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sodium.h"
#include "vault.h"

#if CONFIG_JOLT_STORE_INTERNAL
    #include "storage_internal.h"
#elif CONFIG_JOLT_STORE_ATAES132A
    #include "storage_ataes132a.h"
#endif

bool storage_startup()
{
    /* Configures storage upon first bootup */
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_startup();
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_startup();
#else
    res = false;
#endif
    return res;
}

void storage_stretch_init()
{
#if CONFIG_JOLT_STORE_INTERNAL
    storage_internal_stretch_init();
#elif CONFIG_JOLT_STORE_ATAES132A
    storage_ataes132a_stretch_init();
#endif
}

void storage_stretch( uint256_t hash, int8_t *progress )
{
#if CONFIG_JOLT_STORE_INTERNAL
    storage_internal_stretch( hash, progress );
#elif CONFIG_JOLT_STORE_ATAES132A
    storage_ataes132a_stretch( hash, progress );
#endif
}

bool storage_exists_mnemonic()
{
    /* Checks if a mnemonic has been previously setup on the device.
     * Returns true if mnemonic has been setup; false otherwise*/
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_exists_mnemonic();
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_exists_mnemonic();
#else
    res = false;
#endif
    return res;
}

void storage_set_mnemonic( const uint256_t bin, const uint256_t pin_hash )
{
    /* store binary mnemonic */
#if CONFIG_JOLT_STORE_INTERNAL
    storage_internal_set_mnemonic( bin, pin_hash );
#elif CONFIG_JOLT_STORE_ATAES132A
    storage_ataes132a_set_mnemonic( bin, pin_hash );
#endif
    return;
}

bool storage_get_mnemonic( uint256_t bin, const uint256_t pin_hash )
{
    /* Gets mnemonic for supplied pin hash */
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_get_mnemonic( bin, pin_hash );
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_get_mnemonic( bin, pin_hash );
#else
    res = false;
#endif
    return res;
}

uint32_t storage_get_pin_count()
{
    /* Gets the current PIN counter. This value is monotomicly increasing.
     * Internally, each function should factory reset if the pin count
     * cannot be obtained for some reason.*/
    uint32_t res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_get_pin_count();
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_get_pin_count();
#else
    res = UINT32_MAX; /* Safety default towards failure */
#endif
    return res;
}

void storage_set_pin_count( uint32_t count )
{
    /* Sets the current PIN counter. This value is monotomicly increasing. */
#if CONFIG_JOLT_STORE_INTERNAL
    storage_internal_set_pin_count( count );
#elif CONFIG_JOLT_STORE_ATAES132A
    storage_ataes132a_set_pin_count( count );
#endif
}

uint32_t storage_get_pin_last()
{
    /* Gets the pin counter of the last successful pin attempt.
     */
    uint32_t res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_get_pin_last();
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_get_pin_last();
#else
    res = 0;          /* Safety default towards failure */
#endif
    return res;
}

void storage_set_pin_last( uint32_t count )
{
    /* Gets the pin counter of the last successful pin attempt.
     */
#if CONFIG_JOLT_STORE_INTERNAL
    storage_internal_set_pin_last( count );
#elif CONFIG_JOLT_STORE_ATAES132A
    storage_ataes132a_set_pin_last( count );
#endif
}

bool storage_get_u8( uint8_t *value, const char *namespace, const char *key, uint8_t default_value )
{
    /* Populates [value] from value in storage with [namespace] and [key].
     * If [key] is not found in storage, set [value] to [default_value]
     *
     * Returns true if key was found.
     */
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_get_u8( value, namespace, key, default_value );
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_get_u8( value, namespace, key, default_value );
#else
    res = false;
#endif
    return res;
}

bool storage_set_u8( uint8_t value, const char *namespace, const char *key )
{
    /* Stores [value] into [key]. Primarily used for settings.
     *
     * Returns true on success, false on failure.
     */
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_set_u8( value, namespace, key );
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_set_u8( value, namespace, key );
#else
    res = false;
#endif
    return res;
}

bool storage_get_u16( uint16_t *value, const char *namespace, const char *key, uint16_t default_value )
{
    /* Populates [value] from value in storage with [namespace] and [key].
     * If [key] is not found in storage, set [value] to [default_value]
     *
     * Returns true if key was found.
     */
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_get_u16( value, namespace, key, default_value );
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_get_u16( value, namespace, key, default_value );
#else
    res = false;
#endif
    return res;
}

bool storage_set_u16( uint16_t value, const char *namespace, const char *key )
{
    /* Stores [value] into [key]. Primarily used for settings.
     *
     * Returns true on success, false on failure.
     */
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_set_u16( value, namespace, key );
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_set_u16( value, namespace, key );
#else
    res = false;
#endif
    return res;
}

bool storage_get_u32( uint32_t *value, const char *namespace, const char *key, uint32_t default_value )
{
    /* Populates [value] from value in storage with [namespace] and [key].
     * If [key] is not found in storage, set [value] to [default_value]
     *
     * Returns true if key was found.
     */
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_get_u32( value, namespace, key, default_value );
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_get_u32( value, namespace, key, default_value );
#else
    res = false;
#endif
    return res;
}

bool storage_set_u32( uint32_t value, const char *namespace, const char *key )
{
    /* Stores [value] into [key]. Primarily used for settings.
     *
     * Returns true on success, false on failure.
     */
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_set_u32( value, namespace, key );
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_set_u32( value, namespace, key );
#else
    res = false;
#endif
    return res;
}

bool storage_get_blob( unsigned char *buf, size_t *required_size, const char *namespace, const char *key )
{
    /* Populates [value] from value in storage with [namespace] and [key].
     * If [key] is not found in storage, set [value] to [default_value]
     *
     * Returns true if key was found.
     */
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_get_blob( buf, required_size, namespace, key );
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_get_blob( buf, required_size, namespace, key );
#else
    res = false;
#endif
    return res;
}

bool storage_set_blob( const unsigned char *buf, size_t len, const char *namespace, const char *key )
{
    /* Stores [value] into [key]. Primarily used for settings.
     *
     * Returns true on success, false on failure.
     */
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_set_blob( buf, len, namespace, key );
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_set_blob( buf, len, namespace, key );
#else
    res = false;
#endif
    return res;
}

bool storage_get_str( char *buf, size_t *required_size, const char *namespace, const char *key,
                      const char *default_value )
{
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_get_str( buf, required_size, namespace, key, default_value );
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_get_str( buf, required_size, namespace, key, default_value );
#else
    res = false;
#endif
    return res;
}

bool storage_set_str( const char *str, const char *namespace, const char *key )
{
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_set_str( str, namespace, key );
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_set_str( str, namespace, key );
#else
    res = false;
#endif
    return res;
}

/* Performs the factory reset in a blocking manner */
void storage_factory_reset( bool reset )
{
#if CONFIG_JOLT_STORE_INTERNAL
    storage_internal_factory_reset();
#elif CONFIG_JOLT_STORE_ATAES132A
    storage_ataes132a_factory_reset();
#endif
    if( reset ) { esp_restart(); }
}

bool storage_erase_key( const char *namespace, const char *key )
{
    bool res;
#if CONFIG_JOLT_STORE_INTERNAL
    res = storage_internal_erase_key( namespace, key );
#elif CONFIG_JOLT_STORE_ATAES132A
    res = storage_ataes132a_erase_key( namespace, key );
#else
    res = false;
#endif
    return res;
}
