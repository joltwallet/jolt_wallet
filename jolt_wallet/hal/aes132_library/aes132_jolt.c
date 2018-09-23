#include <stdint.h>
#include <string.h>
#include "aes132_conf.h"
#include "aes132_cmd.h"
#include "aes132_comm_marshaling.h"
#include "aes132_jolt.h"
#include "aes132_mac.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "jolttypes.h"
#include "sodium.h"

static const char TAG[] = "aes132_jolt";
static uint8_t *master_key = NULL;


static struct aes132h_nonce_s *get_nonce() {
    /* Always returns a valid nonce pointer */
    static struct aes132h_nonce_s nonce_obj;
    struct aes132h_nonce_s *nonce = &nonce_obj;

    uint8_t res;
    if(!nonce->valid) {
        ESP_LOGI(TAG, "Nonce Invalid; Refreshing Nonce");
        res = aes132_nonce(nonce, NULL); 
        if( res ) {
            ESP_LOGE(TAG, "Error %02X generating new nonce", res);
            esp_restart();
        }
        else {
            ESP_LOGI(TAG, "Nonce Refreshed");
        }
    }
    return nonce;
}

uint8_t aes132_jolt_setup() {
    /*
     * In both cases:
     *     * Allocates secure space for the master key on the heap.
     *     * Issue a nonce comman to aes132.
     * If device is unlocked:
     *     * Generate Master Key from ESP32 Entropy
     *     * Configure Device
     *     * Encrypt and Backup Master Key to MasterUserZone
     *     * Write key to slot 0
     *     * Lock Device
     * If device is locked:
     *     * Load master key from local storage;
     *         * If not available, Load & decrypt Master key from MasterUserZone
     */
    uint8_t res;
    if( NULL==master_key ) {
        master_key = sodium_malloc(16);
    }
    if( NULL==master_key ) {
        ESP_LOGE(TAG, "Unable to allocate space for the ATAES132A Master Key");
        esp_restart();
    }

    /* Check if the device is locked, if not locked generate a new master key */ 
    ESP_LOGI(TAG, "Checking if device is locked");
    bool locked;
    res = aes132_check_configlock(&locked);
    if( res ) {
        ESP_LOGE(TAG, "Unable to check if ATAES132A is locked");
        esp_restart();
    }

    if( locked ) {
        /* Attempt to load key from encrypted spi flash */
        /* Loads the key from storage */
        // todo
        /* If not found in storage, check the Master UserZone */
        {
            uint8_t rx[16];
            res = aes132m_read_memory(sizeof(rx), AES132_USER_ZONE_ADDR(0), rx);
            ESP_LOGI(TAG, "Read memory result: %d", res);
            ESP_LOGI(TAG, "Confidential; "
                    "ATAES132A Master Key Backup Response: 0x"
                    "%02x%02x%02x%02x%02x%02x%02x%02x"
                    "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                    rx[0], rx[1], rx[2], rx[3], rx[4], rx[5], rx[6], rx[7],
                    rx[8], rx[9], rx[10], rx[11], rx[12], rx[13], rx[14],
                    rx[15]);
            //todo decrypt rx; here just identity
            memcpy(master_key, rx, 16);
        }
    }
    else {
        //aes132_check_manufacturing_id();
        /* Generate new master key 
         * We cannot use ataes132a for additional entropy since its unlocked */
        /* aes132 will generate non-random 0xA5 bytes until the LockConfig
         * reigster is locked, so we cannot use it for additional entropy,
         * so we cannot use it for additional entropy.
         * todo: investigate more sources of entropy. */
        for( uint8_t i=0; i<4; i++ ) {
            uint32_t entropy = randombytes_random();
            memcpy(&((uint32_t*)master_key)[i], &entropy, sizeof(uint32_t));
#ifdef UNIT_TESTING
            ((uint32_t*)master_key)[i] = 0x11111111;
#endif
        }

        ESP_LOGI(TAG, "Confidential; ATAES132A Master Key: 0x"
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                master_key[0], master_key[1], master_key[2],
                master_key[3], master_key[4], master_key[5],
                master_key[6], master_key[7], master_key[8],
                master_key[9], master_key[10], master_key[11],
                master_key[12], master_key[13], master_key[14],
                master_key[15]);

        /* Encrypt Key with ESP32 Storage Engine Key */
        // todo; replace this; this is an identity placeholder
        uint8_t enc_master_key[16];
        memcpy(enc_master_key, master_key, sizeof(enc_master_key));

        /* Backup Encrypted Key to Device*/
        // Make sure the zone config is in a state where we can write to
        // UserZone 0
        ESP_LOGI(TAG, "Reseting Master UserZone Config so the master key can "
                "be written in plaintext");
        aes132_reset_master_zoneconfig();
        ESP_LOGI(TAG, "Writing encrypted Master Key backup to UserZone0");
        res = aes132m_write_memory(sizeof(enc_master_key),
                AES132_USER_ZONE_ADDR(AES132_KEY_ID_MASTER), enc_master_key);
        ESP_LOGI(TAG, "Write memory result: %d", res);

        /* Confirm Backup */
        {
            uint8_t rx[16];
            res = aes132m_read_memory(sizeof(enc_master_key),
                    AES132_USER_ZONE_ADDR(AES132_KEY_ID_MASTER),
                    rx);
            ESP_LOGI(TAG, "Confirming UserZone 0 contents");
            ESP_LOGI(TAG, "Read memory result: %d", res);
            ESP_LOGI(TAG, "Confidential; "
                    "ATAES132A Master Key Backup Response: 0x"
                    "%02x%02x%02x%02x%02x%02x%02x%02x"
                    "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                    rx[0], rx[1], rx[2], rx[3], rx[4], rx[5], rx[6], rx[7],
                    rx[8], rx[9], rx[10], rx[11], rx[12], rx[13], rx[14],
                    rx[15]);

            ESP_ERROR_CHECK(memcmp(enc_master_key, rx, sizeof(rx)));
        }
        /* Write Key to Key0 */
        ESP_LOGI(TAG, "Writing Master Key to Key %d", AES132_KEY_ID_MASTER);
        res = aes132m_write_memory(16,
                AES132_KEY_ADDR(AES132_KEY_ID_MASTER), master_key);
        ESP_LOGI(TAG, "Write memory to 0x%04X result: %d",
                AES132_KEY_ADDR(AES132_KEY_ID_MASTER), res);
        
        /* Configure Device */
        ESP_LOGI(TAG, "Configuring Device");
        aes132_write_chipconfig();
        aes132_write_counterconfig();
        aes132_write_keyconfig();
        aes132_write_zoneconfig();

        /* Lock Device */
#ifdef UNIT_TESTING
        ESP_LOGI(TAG, "Not locking device since we are in unit_testing mode");
        // Do Nothing; Don't actually lock device
#else
        //lock_device();
        ESP_LOGE(TAG, "While debugging, if lock_device wasn't commented out, "
                "itd be locked.\n");
        // todo: test
#endif

    }

    sodium_mprotect_noaccess(master_key);
    get_nonce(); // Prime the Nonce mechanism
    return res;
}

uint8_t aes132_pin_load_keys(const uint8_t *key) {
    /* Assumes 256-bit input key */
    uint8_t res = 0xFF;
    struct aes132h_nonce_s *nonce = get_nonce();

    CONFIDENTIAL unsigned char child_key[crypto_auth_hmacsha512_BYTES];
    for(uint8_t key_id = AES132_KEY_ID_PIN(0); key_id<16; key_id++) {
        crypto_auth_hmacsha512(child_key, &key_id, 1, key);
        /* Different derived keys are stored in different slots.
         * If somehow partial data can be recovered from keyslots, this
         * prevents the data from being overly redundant */
        ESP_LOGI(TAG, " Loading slot %d with child key "
                "%02X %02X %02X %02X %02X %02X %02X %02X "
                "%02X %02X %02X %02X %02X %02X %02X %02X", key_id,
                child_key[0], child_key[1], child_key[2],
                child_key[3], child_key[4], child_key[5],
                child_key[6], child_key[7], child_key[8],
                child_key[9], child_key[10], child_key[11],
                child_key[12], child_key[13], child_key[14],
                child_key[15]);
        res = aes132_key_load(master_key, child_key, key_id, nonce);
        if( res ) {
            ESP_LOGE(TAG, "Failed to slot PIN in key slot %d. "
                    "Error Code: 0x%02X", key_id, res);
            goto exit;
        }
    }
exit:
    sodium_memzero(child_key, sizeof(child_key));
    return res;
}

uint8_t aes132_pin_attempt(const uint8_t *key, uint32_t *counter) {
    /* No matter what, will return a counter value that can be used
     * to determine whether or not to wipe the device */
    uint8_t res = 0;
    uint32_t cum_counter = 0;
    uint8_t attempt_slot = 0xFF; // Sentinel Value
    struct aes132h_nonce_s *nonce = get_nonce();
    CONFIDENTIAL unsigned char child_key[crypto_auth_hmacsha512_BYTES];

    /* Gather counter values, determine which slot to attempt */
    for(uint8_t counter_id=AES132_KEY_ID_PIN(0); counter_id < 16; counter_id++) {
        // For safety, default to maximum value
        uint32_t count = AES132_COUNTER_MAX;
        res = aes132_counter(master_key, &count, counter_id, nonce);
        if( res ) {
            ESP_LOGE(TAG, "Error attempting to read counter %d. "
                    "RetCode 0x%02X.", counter_id, res);
        }
        if( count < AES132_COUNTER_MAX) {
            attempt_slot = counter_id;
        }
        cum_counter += count;
    }
    if( 0xFF == attempt_slot ) {
        ESP_LOGE(TAG, "Device key-use completely exhausted");
        res = AES132_DEVICE_RETCODE_COUNT_ERROR; 
        goto exit;
    }
    /* attempt authentication */
    crypto_auth_hmacsha512(child_key, &attempt_slot, 1, key);
    res = aes132_auth(child_key, attempt_slot, nonce);
    if( res ) {
        ESP_LOGE(TAG, "Failed authenticated key_slot %d. Child Key: "
                "%02X %02X %02X %02X %02X %02X %02X %02X "
                "%02X %02X %02X %02X %02X %02X %02X %02X", attempt_slot,
                child_key[0], child_key[1], child_key[2],
                child_key[3], child_key[4], child_key[5],
                child_key[6], child_key[7], child_key[8],
                child_key[9], child_key[10], child_key[11],
                child_key[12], child_key[13], child_key[14],
                child_key[15]);
        goto exit;
    }
    else {
        ESP_LOGI(TAG, "Successfully authenticated with key_slot %d",
                attempt_slot);
    }
exit:
    *counter = cum_counter;
    sodium_memzero(child_key, sizeof(child_key));
    return res;
}

uint8_t aes132_create_stretch_key() {
    /* Warning; overwriting an existing stretch key will cause permament
     * loss of mnemonic */
    uint8_t res;
    struct aes132h_nonce_s *nonce = get_nonce();
    res = aes132_key_create( AES132_KEY_ID_STRETCH, nonce );
    if( AES132_DEVICE_RETCODE_SUCCESS != res ) {
        ESP_LOGE(TAG, "Failed creating stretch key with retcode %02X\n", res);
    }
    return res;
}

uint8_t aes132_stretch(uint8_t *data, const uint8_t data_len, uint32_t n_iter ) {
    /* Hardware bound key stretching. We limit PIN bruteforcing (in the event 
     * of a complete esp32 compromise) by how quickly the ataes132a can respond
     * to encrypt commands. Compared to conventional keystretching, PIN 
     * brute forcing attempts must be done serially, and do not benefit from 
     * more compute power.
     *
     * Runs AES128 Encryption over the data n_iter times.
     *
     * We do not verify the MAC of the returned ciphertext; unnecessary for
     * our application.
     */
    uint8_t res = 0;
    for(uint32_t i=0; i < n_iter; i++) {
        struct aes132h_nonce_s *nonce = get_nonce();
        res = aes132_encrypt(data, data_len, AES132_KEY_ID_STRETCH,
                data, NULL, nonce);
        if( AES132_DEVICE_RETCODE_SUCCESS != res ) {
            ESP_LOGE(TAG, "Failed on iteration %d with retcode %02X\n", i, res);
            break;
        }
    }
    return res;
}


#if 0
bool aes132_pin_attempt(uint8_t *guess, uint8_t *n_attempts) {
    /* Attempts a pin hash. If criteria is met, perform factory reset
     * *n_attempts - out - number of attempts tried.
     * *guess - out - 16 bytes (128-bit)
     *
     * returns true on success, false on failure.
     */
}
#endif
