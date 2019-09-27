//#define LOG_LOCAL_LEVEL 4

#include "aes132_jolt.h"
#include <stdint.h>
#include <string.h>
#include "aes132_cmd.h"
#include "aes132_comm_marshaling.h"
#include "aes132_conf.h"
#include "aes132_mac.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "hal/storage/storage.h"
#include "jolt_helpers.h"
#include "jolttypes.h"
#include "sodium.h"

static const char TAG[]    = "aes132_jolt";
static uint8_t *master_key = NULL;

#define CEIL_DIV( x, y ) ( ( x + y - 1 ) / y )

static struct aes132h_nonce_s *get_nonce()
{
    /* Always returns a valid nonce pointer */
    static struct aes132h_nonce_s nonce_obj;
    struct aes132h_nonce_s *nonce = &nonce_obj;

    if( !nonce->valid ) {
        uint8_t res;
        ESP_LOGI( TAG, "Nonce Invalid; Refreshing Nonce" );
        res = aes132_nonce( nonce, NULL );
        if( res ) {
            ESP_LOGE( TAG, "Error generating new nonce (0x%02X)", res );
            esp_restart();
        }
        else {
            ESP_LOGI( TAG, "Nonce Refreshed" );
        }
    }
    return nonce;
}

uint8_t aes132_jolt_setup()
{
    uint8_t res;
    if( NULL == master_key ) { master_key = sodium_malloc( 16 ); }
    if( NULL == master_key ) {
        ESP_LOGE( TAG, "Unable to allocate space for the ATAES132A Master Key" );
        esp_restart();
    }

    /* Check if the device is locked, if not locked generate a new master key */
    ESP_LOGI( TAG, "Checking if device is locked" );
    bool locked;
    res = aes132_check_configlock( &locked );
    if( res ) {
        ESP_LOGE( TAG, "Unable to check if ATAES132A is locked (0x%02X)", res );
        esp_restart();
    }
    ESP_LOGD( TAG, "ATAES132a is %slocked.", locked ? "" : "NOT " );

#if CONFIG_JOLT_AES132_LOCK
    if( locked ) {
#else
    /* See if we have a locally stored master key as a proxy for checking
     * if locked.*/
    size_t required_size;
    if( storage_get_blob( NULL, &required_size, "aes132", "master" ) ) {
#endif
        ESP_LOGI( TAG, "Using existing keys" );
        /* Attempt to load key from encrypted spi flash */

        size_t required_size = 16;
        /* Load the key from storage */
        if( !storage_get_blob( master_key, &required_size, "aes132", "master" ) ) {
            ESP_LOGE( TAG, "Cannot find local master key; attempting to "
                           "recover from device." );
            /* Check the Master UserZone */
            uint8_t rx[16];
            res = aes132_blockread( rx, AES132_USER_ZONE_ADDR( 0 ), sizeof( rx ) );
            if( res ) {
                ESP_LOGE( TAG, "Failed to blockread master key (0x%02X).", res );
                esp_restart();
            }
            ESP_LOGI( TAG, "Read memory result: %d", res );
            ESP_LOGI( TAG,
                      "Confidential; "
                      "ATAES132A Master Key Backup Response: 0x"
                      "%02x%02x%02x%02x%02x%02x%02x%02x"
                      "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                      rx[0], rx[1], rx[2], rx[3], rx[4], rx[5], rx[6], rx[7], rx[8], rx[9], rx[10], rx[11], rx[12],
                      rx[13], rx[14], rx[15] );
            // todo decrypt rx using esp32 secret; here just identity
            memcpy( master_key, rx, 16 );
        }
    }
    else {
        ESP_LOGI( TAG, "Setting up new keys" );
        /* Generate new master key
         * We cannot use ataes132a for additional entropy since its unlocked */
        /* aes132 will generate non-random 0xA5 bytes until the LockConfig
         * reigster is locked, so we cannot use it for additional entropy,
         * so we cannot use it for additional entropy. */
#ifdef UNIT_TESTING
        {
            // deterministic key for easier unit testing
            const uint8_t dummy[] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
                                     0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

            memcpy( master_key, dummy, 16 );
        }
#else
        jolt_random( master_key, 16 );
#endif

        ESP_LOGI( TAG,
                  "Confidential; ATAES132A Master Key: 0x"
                  "%02x%02x%02x%02x%02x%02x%02x%02x"
                  "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                  master_key[0], master_key[1], master_key[2], master_key[3], master_key[4], master_key[5],
                  master_key[6], master_key[7], master_key[8], master_key[9], master_key[10], master_key[11],
                  master_key[12], master_key[13], master_key[14], master_key[15] );

        /* Locally store master key */
        if( !storage_set_blob( master_key, 16, "aes132", "master" ) ) {
            ESP_LOGE( TAG, "Error trying to store master key to NVS" );
            esp_restart();
        }

        /* Encrypt Key with ESP32 Storage Engine Key */
        // todo; replace this; this is an identity placeholder
        uint8_t enc_master_key[16];
        memcpy( enc_master_key, master_key, sizeof( enc_master_key ) );

        /* Backup Encrypted Key to Device*/
        // Make sure the zone config is in a state where we can write to
        // UserZone 0
        ESP_LOGI( TAG, "Reseting Master UserZone Config so the master key can "
                       "be written in plaintext" );
        aes132_reset_master_zoneconfig();
        ESP_LOGI( TAG, "Writing encrypted Master Key backup to UserZone0" );
        res = aes132m_write_memory( sizeof( enc_master_key ), AES132_USER_ZONE_ADDR( AES132_KEY_ID_MASTER ),
                                    enc_master_key );
        if( res ) {
            ESP_LOGE( TAG, "Failed to backup masterkey to userzone0 (0x%02X).", res );
            esp_restart();
        }

        /* Confirm AES132 Backup */
        {
            uint8_t rx[16];
            res = aes132m_read_memory( sizeof( enc_master_key ), AES132_USER_ZONE_ADDR( AES132_KEY_ID_MASTER ), rx );
            if( res ) {
                ESP_LOGE( TAG, "Failed to readback masterkey backup (0x%02X).", res );
                esp_restart();
            }
            ESP_LOGI( TAG,
                      "Confidential; "
                      "ATAES132A Master Key Backup Response: 0x"
                      "%02x%02x%02x%02x%02x%02x%02x%02x"
                      "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                      rx[0], rx[1], rx[2], rx[3], rx[4], rx[5], rx[6], rx[7], rx[8], rx[9], rx[10], rx[11], rx[12],
                      rx[13], rx[14], rx[15] );

            ESP_ERROR_CHECK( memcmp( enc_master_key, rx, sizeof( rx ) ) );
        }

        /* Write Master Key to Key0 */
        ESP_LOGI( TAG, "Writing Master Key to Key %d (Address: 0x%04X)", AES132_KEY_ID_MASTER,
                  AES132_KEY_ADDR( AES132_KEY_ID_MASTER ) );
        res = aes132m_write_memory( 16, AES132_KEY_ADDR( AES132_KEY_ID_MASTER ), master_key );
        if( res ) {
            ESP_LOGE( TAG, "Failed to write master key to Key %d (0X%02X).", AES132_KEY_ID_MASTER, res );
            esp_restart();
        }

        /* Configure Device */
        ESP_LOGI( TAG, "Configuring Device" );

        res = aes132_write_chipconfig();
        if( res ) {
            ESP_LOGE( TAG, "Failed configuring chip (0x%02X).", res );
            esp_restart();
        }

        res = aes132_write_counterconfig();
        if( res ) {
            ESP_LOGE( TAG, "Failed configuring counters (0x%02X).", res );
            esp_restart();
        }

        res = aes132_write_keyconfig();
        if( res ) {
            ESP_LOGE( TAG, "Failed configuring keys (0x%02X).", res );
            esp_restart();
        }

        res = aes132_write_zoneconfig();
        if( res ) {
            ESP_LOGE( TAG, "Failed configuring zones (0x%02X).", res );
            esp_restart();
        }

        /* Lock Device */
        ESP_LOGI( TAG, "AES132 Config Complete." );
#if CONFIG_JOLT_AES132_LOCK && !UNIT_TESTING
        ESP_LOGI( TAG, "***Locking device.***" );
        assert( 0 == aes132_lock_device() );
#else
        ESP_LOGE( TAG, "Not locking device. UNSAFE FOR CONSUMER." );
#endif
    }

    sodium_mprotect_noaccess( master_key );
    get_nonce();  // Prime the Nonce mechanism
    return res;
}

uint8_t aes132_pin_load_keys( const uint8_t *key )
{
    /* Assumes 256-bit input key */
    uint8_t res                   = 0xFF;
    struct aes132h_nonce_s *nonce = get_nonce();

    CONFIDENTIAL unsigned char child_key[crypto_auth_hmacsha512_BYTES];
    for( uint8_t key_id = AES132_KEY_ID_PIN( 0 ); key_id < 16; key_id++ ) {
        crypto_auth_hmacsha512( child_key, &key_id, 1, key );
        /* Different derived keys are stored in different slots.
         * If somehow partial data can be recovered from keyslots, this
         * prevents the data from being overly redundant */
        ESP_LOGI( TAG,
                  " Loading slot %d with child key "
                  "%02X %02X %02X %02X %02X %02X %02X %02X "
                  "%02X %02X %02X %02X %02X %02X %02X %02X",
                  key_id, child_key[0], child_key[1], child_key[2], child_key[3], child_key[4], child_key[5],
                  child_key[6], child_key[7], child_key[8], child_key[9], child_key[10], child_key[11], child_key[12],
                  child_key[13], child_key[14], child_key[15] );
        res = aes132_key_load( master_key, child_key, key_id, nonce );
        if( res ) {
            ESP_LOGE( TAG, "Failed to slot PIN in key slot %d (0x%02X).", key_id, res );
            goto exit;
        }
    }
exit:
    sodium_memzero( child_key, sizeof( child_key ) );
    return res;
}

uint8_t aes132_pin_load_zones( const uint8_t *key, const uint8_t *secret )
{
    /* Assumes 256-bit input key */
    uint8_t res                   = 0xFF;
    struct aes132h_nonce_s *nonce = get_nonce();
    CONFIDENTIAL unsigned char child_key[crypto_auth_hmacsha512_BYTES];
    CONFIDENTIAL uint256_t zone_secret;

    assert( crypto_auth_hmacsha512_KEYBYTES == 32 );

    for( uint8_t key_id = AES132_KEY_ID_PIN( 0 ); key_id < 16; key_id++ ) {
        crypto_auth_hmacsha512( child_key, &key_id, sizeof( key_id ), key );
        /* Different derived keys are stored in different slots.
         * If somehow partial data can be recovered from keyslots, this
         * prevents the data from being overly redundant */
        ESP_LOGI( TAG,
                  "Authorizing slot %d with child key "
                  "%02X %02X %02X %02X %02X %02X %02X %02X "
                  "%02X %02X %02X %02X %02X %02X %02X %02X",
                  key_id, child_key[0], child_key[1], child_key[2], child_key[3], child_key[4], child_key[5],
                  child_key[6], child_key[7], child_key[8], child_key[9], child_key[10], child_key[11], child_key[12],
                  child_key[13], child_key[14], child_key[15] );

        // Authorize r/w zone
        res = aes132_auth( child_key, key_id, nonce );
        if( res ) {
            ESP_LOGE( TAG, "Failed to auth key_id %d (0x%02X).", key_id, res );
            goto exit;
        }

        // Derive and write secret to zone
        for( uint8_t i = 0; i < 32; i++ ) { zone_secret[i] = secret[i] ^ child_key[i]; }
        res = aes132m_write_memory( 32, AES132_USER_ZONE_ADDR( key_id ), zone_secret );
        if( res ) {
            ESP_LOGE( TAG, "Failed to store secret in zone %d (0x%02X).", key_id, res );
            goto exit;
        }
    }
exit:
    sodium_memzero( child_key, sizeof( child_key ) );
    sodium_memzero( zone_secret, sizeof( zone_secret ) );
    return res;
}

uint8_t aes132_pin_attempt( const uint8_t *key, uint32_t *counter, uint8_t *secret )
{
    /* Derives child key for the last valid key slot and attempt authentication.
     * No matter what, will return a counter value that can be used
     * to determine whether or not to wipe the device.
     *
     * Returns 0x00 on authorization success,
     * 0x40 on authorization failure (because of invalid inbound mac).
     *
     * If key is NULL, only populate counter.*/
    uint8_t res                   = 0;
    uint32_t cum_counter          = 0;
    uint8_t attempt_slot          = 0xFF;  // Sentinel Value
    struct aes132h_nonce_s *nonce = get_nonce();
    CONFIDENTIAL unsigned char child_key[crypto_auth_hmacsha512_BYTES];

    assert( crypto_auth_hmacsha512_BYTES >= 32 );

    /* Gather counter values, determine which slot to attempt */
    for( uint8_t counter_id = AES132_KEY_ID_PIN( 0 ); counter_id < 16; counter_id++ ) {
        // For safety, default to maximum value
        uint32_t count = AES132_COUNTER_MAX;
        res            = aes132_counter( master_key, &count, counter_id, nonce );
        if( res ) { ESP_LOGE( TAG, "Error attempting to read counter %d (0x%02X).", counter_id, res ); }
        cum_counter += count;
        if( count < AES132_COUNTER_MAX && 0xFF == attempt_slot ) {
            attempt_slot = counter_id;
            if( NULL == counter ) {
                // Don't bother reading the rest if we aren't returning
                // cumulative value
                break;
            }
        }
    }
    if( 0xFF == attempt_slot ) {
        ESP_LOGE( TAG, "Device key-use completely exhausted" );
        res = AES132_DEVICE_RETCODE_COUNT_ERROR;
        goto exit;
    }
    /* attempt authentication */
    if( NULL == key ) { goto exit; }
    crypto_auth_hmacsha512( child_key, &attempt_slot, sizeof( attempt_slot ), key );
    res = aes132_auth( child_key, attempt_slot, nonce );
    if( res ) {
        ESP_LOGE( TAG,
                  "Failed authenticated key_slot %d (0x%02X). Child Key: "
                  "%02X %02X %02X %02X %02X %02X %02X %02X "
                  "%02X %02X %02X %02X %02X %02X %02X %02X",
                  attempt_slot, res, child_key[0], child_key[1], child_key[2], child_key[3], child_key[4],
                  child_key[5], child_key[6], child_key[7], child_key[8], child_key[9], child_key[10], child_key[11],
                  child_key[12], child_key[13], child_key[14], child_key[15] );
        goto exit;
    }
    ESP_LOGI( TAG, "Successfully authenticated with key_slot %d", attempt_slot );
    {
        CONFIDENTIAL uint8_t zone_secret[32];
        res = aes132_blockread( zone_secret, AES132_USER_ZONE_ADDR( attempt_slot ), sizeof( zone_secret ) );
        if( res ) {
            ESP_LOGE( TAG, "Error reading from user zone (0x%02X)", res );
            goto exit;
        }

        // xor child_key and secret together
        for( uint8_t i = 0; i < 32; i++ ) { secret[i] = zone_secret[i] ^ child_key[i]; }
        ESP_LOGD( TAG,
                  "Re-assembled ATAES132a Secret: "
                  "%02X %02X %02X %02X %02X %02X %02X %02X "
                  "%02X %02X %02X %02X %02X %02X %02X %02X "
                  "%02X %02X %02X %02X %02X %02X %02X %02X "
                  "%02X %02X %02X %02X %02X %02X %02X %02X ",
                  secret[0], secret[1], secret[2], secret[3], secret[4], secret[5], secret[6], secret[7], secret[8],
                  secret[9], secret[10], secret[11], secret[12], secret[13], secret[14], secret[15], secret[16],
                  secret[17], secret[18], secret[19], secret[20], secret[21], secret[22], secret[23], secret[24],
                  secret[25], secret[26], secret[27], secret[28], secret[29], secret[30], secret[31] );
        sodium_memzero( zone_secret, sizeof( zone_secret ) );
    }

exit:
    if( NULL != counter ) {
        *counter = cum_counter;
        ESP_LOGI( TAG, "Cumulative PIN Counter: %d", cum_counter );
    }
    sodium_memzero( child_key, sizeof( child_key ) );
    return res;
}

uint8_t aes132_pin_counter( uint32_t *counter )
{
    /* Gets the cumulative counter of all pin keyslots */
    uint8_t res;
    uint32_t cum_counter          = 0;
    struct aes132h_nonce_s *nonce = get_nonce();

    /* Gather counter values, determine which slot to attempt */
    for( uint8_t counter_id = AES132_KEY_ID_PIN( 0 ); counter_id < 16; counter_id++ ) {
        // For safety, default to maximum value
        uint32_t count = AES132_COUNTER_MAX;
        res            = aes132_counter( master_key, &count, counter_id, nonce );
        if( res ) { ESP_LOGE( TAG, "Error attempting to read counter %d (0x%02X).", counter_id, res ); }
        cum_counter += count;
    }
    *counter = cum_counter;
    ESP_LOGI( TAG, "Cumulative PIN Counter: %d", cum_counter );
    return res;
}

uint8_t aes132_create_stretch_key()
{
    /* Warning; overwriting an existing stretch key will cause permament
     * loss of mnemonic */
    uint8_t res;
    struct aes132h_nonce_s *nonce = get_nonce();
    res                           = aes132_key_create( AES132_KEY_ID_STRETCH, nonce );
    if( AES132_DEVICE_RETCODE_SUCCESS != res ) {
        ESP_LOGE( TAG, "Failed creating stretch key with retcode %02X\n", res );
    }
    return res;
}

uint8_t aes132_stretch( uint8_t *data, const uint8_t data_len, uint32_t n_iter, int8_t *progress )
{
    /* Hardware bound key stretching. We limit PIN bruteforcing (in the event
     * of a complete esp32 compromise) by how quickly the ataes132a can respond
     * to encrypt commands. Compared to conventional keystretching, PIN
     * brute forcing attempts must be done serially, and do not benefit from
     * more compute power.
     *
     * Runs AES128 ECB (16-byte block) over the data n_iter times.
     *
     * We do not verify the MAC of the returned ciphertext; unnecessary for
     * our application.
     */
    uint8_t res             = 0;
    uint8_t j               = 0;
    uint32_t total_iter     = CEIL_DIV( data_len, 16 ) * n_iter;
    uint32_t total_iter_ctr = 0;
    // Even though Legacy doesn't use a Nonce, it requires a valid Nonce
    // on device.
    get_nonce();

    ESP_LOGD( TAG, "Total Stretch Iterations: %d", total_iter );

    if( NULL == progress ) ESP_LOGD( TAG, "Progress is NULL; will not be updated." );

    ESP_LOGD( TAG,
              "Stretch Input (first 128-bits): "
              "%02X %02X %02X %02X %02X %02X %02X %02X "
              "%02X %02X %02X %02X %02X %02X %02X %02X",
              data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10],
              data[11], data[12], data[13], data[14], data[15] );

    total_iter /= 100;  // Progress is represented from 0-100, not 0.0-1.0
    do {
        CONFIDENTIAL uint8_t buf[16] = {0};  // Separate buffer for when input % 16 != 0
        uint32_t buf_len;
        buf_len = data_len - j;
        if( buf_len > 16 ) buf_len = 16;
        memcpy( buf, &data[j], buf_len );
        for( uint32_t i = 0; i < n_iter; i++ ) {
            res = aes132_legacy( AES132_KEY_ID_STRETCH, buf );
            if( AES132_DEVICE_RETCODE_SUCCESS != res ) {
                ESP_LOGE( TAG, "Failed on iteration %d with retcode %02X\n", i, res );
                sodium_memzero( buf, sizeof( buf ) );
                goto exit;
            }
            total_iter_ctr++;
            if( NULL != progress ) {
                uint8_t tmp;
                tmp = total_iter_ctr / total_iter;
                if( tmp <= 100 ) *progress = tmp;
            }
        }
        memcpy( &data[j], buf, 16 );
        j += buf_len;
        sodium_memzero( buf, sizeof( buf ) );
    } while( j < data_len );

    ESP_LOGD( TAG,
              "Stretch Output (first 128-bits): "
              "%02X %02X %02X %02X %02X %02X %02X %02X "
              "%02X %02X %02X %02X %02X %02X %02X %02X",
              data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10],
              data[11], data[12], data[13], data[14], data[15] );

exit:
    return res;
}
