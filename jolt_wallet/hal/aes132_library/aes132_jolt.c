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
static struct aes132h_nonce_s *get_nonce();
/* Used to mirror the internal MacCount of the ataes132a device.
 * The ataes132a MacCount gets zero'd when:
 *     * Nonce command is executed
 *     * A MAC compare operation fails.
 *     * MacCount reaches the maximum count.
 *     * A Reset event occurs: 
 *          * Power-Up (see Appendix L ChipState = Power-Up)
 *          * Wake-Up from Sleep (see Appendix L. ChipState = Wake-Up from Sleep)
 *          * Reset command (see Section 7.22 Reset Command)
 *          * Security Tamper is activated, causing the hardware to reset
 * The ataes132a MacCount gets incremented when:
 *     * prior to any MAC computation being performed
 * E.g. the value that will be used for calculating the first MAC of the first 
 * command after the Nonce command is MAC=1.
 * Can be read via the INFO command.
 * For more information, see Page 107
 */

/* Static Functions */
static uint8_t mac_incr();
static uint8_t aes132_nonce(struct aes132h_nonce_s *nonce,
        const uint8_t *in_seed);


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

#define MAC_COUNT_LOCKSTEP_CHECK true
static uint8_t mac_incr() {
    uint8_t *mac_count;
    struct aes132h_nonce_s *nonce = get_nonce();
    mac_count = &(nonce->value[12]);

#if MAC_COUNT_LOCKSTEP_CHECK
    {
        // Read the aes132m mac_count to make sure we are in lock-step
        uint8_t device_count;
        aes132_mac_count(&device_count);
        if( device_count != *mac_count ) {
            ESP_LOGE(TAG, "mac_count desyncronized before incrementing.\n"
                    "Device=%d; Local=%d",
                    device_count, *mac_count);
        }
    }
#endif
    (*mac_count)++;
    if( UINT8_MAX == *mac_count ) {
        // todo: error handling, nonce sync?
        ESP_LOGD(TAG, "local mac_count maxed out, issuing rand Nonce");
        nonce->valid = false;
    }
    return *mac_count;
}

static uint8_t aes132_nonce(struct aes132h_nonce_s *nonce,
        const uint8_t *in_seed) {
    /* Sends NONCE cmd to aes132.
     * Random is only populated if in_seed is NULL.
     * Populates the global nonce variable
     * in_seed should be 12 bytes.
     * */
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};
    uint8_t cmd, mode;
    uint16_t param1, param2;
    uint8_t data[12] = { 0 };
    

    cmd = AES132_OPCODE_NONCE;
    if( NULL == in_seed) {
        mode = 0x01; // Generate a random Nonce using internal RNG
    }
    else {
        mode = 0x00; // Use inbound seed for nonce
        memcpy(data, in_seed, sizeof(data));
    }
    //mode |= AES132_EEPROM_RNG_UPDATE;
    mode |= 0x02;
    param1 = 0x0000; // Always 0
    param2 = 0x0000; // Always 0
    res = aes132m_execute(cmd, mode, param1, param2,
            12, data, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( res ) {
        /* Error */
        ESP_LOGE(TAG, "Nonce returncode: %02X",
                rx_buffer[AES132_RESPONSE_INDEX_RETURN_CODE]);
    }
    else {
        /* Success */
        // Stop! Maybe need to compute nonce from the returned data
        struct aes132h_nonce_in_out nonce_param;
        nonce_param.mode    = mode;
        nonce_param.in_seed = data;
        nonce_param.random  = &rx_buffer[AES132_RESPONSE_INDEX_DATA];
        nonce_param.nonce   = nonce;
        res = aes132h_nonce(&nonce_param);
        //memcpy(nonce->value, &rx_buffer[AES132_RESPONSE_INDEX_DATA], 12);
        ESP_LOGI(TAG, "Local Nonce updated to %02X %02X %02X %02X %02X "
                "%02X %02X %02X %02X %02X %02X %02X",
                nonce->value[0], nonce->value[1], nonce->value[2],
                nonce->value[3], nonce->value[4], nonce->value[5],
                nonce->value[6], nonce->value[7], nonce->value[8],
                nonce->value[9], nonce->value[10], nonce->value[11] );
        if( mode & 0x01 ) {
            ESP_LOGI(TAG, "Setting Nonce Random Flag True");
            nonce->random = true;
        }
        else {
            ESP_LOGI(TAG, "Setting Nonce Random Flag False");
            nonce->random = false;
        }
        nonce->value[12] = 0;
        ESP_LOGI(TAG, "Setting Nonce Valid Flag True");
        nonce->valid = true;
    }
    return res;
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

static uint8_t aes132_key_create(uint8_t key_id) {
    /* Only used to create the stretch key */
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = { 0 };
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = { 0 };
    uint8_t cmd, mode;
    uint16_t param1, param2;
    struct aes132h_nonce_s *nonce = get_nonce();

    // Increment MacCount before operation. 
    mac_incr();

    // todo: mac stuff
    cmd = AES132_OPCODE_KEY_CREATE;
    mode = 0x07; 
    param1 = key_id;
    param2 = 0x0000;
    res = aes132m_execute(cmd, mode, param1, param2,
            0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( res ) {
        nonce->valid = false;
    }
    return res;
}

uint8_t aes132_create_stretch_key() {
    /* Warning; overwriting an existing stretch key will cause permament
     * loss of mnemonic */
    uint8_t res;
    res = aes132_key_create( AES132_KEY_ID_STRETCH );
    if( AES132_DEVICE_RETCODE_SUCCESS != res ) {
        ESP_LOGE(TAG, "Failed creating stretch key with retcode %02X\n", res);
    }
    return res;
}

static uint8_t aes132_encrypt(const uint8_t *in, uint8_t len, uint8_t key_id,
        uint8_t *out_data, uint8_t *out_mac) {
    /* Only used in key stretching!
     *
     * Encrypts upto 32 bytes of data (*in with length len) using key_id
     * returns cipher text (*out)
     * len must be <=32.
     * out_data must be able to handle 16 bytes (<=16byte in) or 
     * 32 bytes (<=32 byte in)
     *
     * todo: something with the output MAC*/
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = { 0 };
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = { 0 };
    uint8_t cmd, mode;
    uint16_t param1, param2;
    struct aes132h_nonce_s *nonce = get_nonce();

    uint8_t ciphertext_len;
    if( len > 32 ) {
        ESP_LOGE(TAG, "ENCRYPT accepts a maximum of 32 bytes. "
                "%d bytes were provided.", len);
        return AES132_DEVICE_RETCODE_PARSE_ERROR; // todo: better error
    }
    else if( len > 16 ) {
        ciphertext_len = 32;
    }
    else if(len > 0) {
        ciphertext_len = 16;
        return AES132_DEVICE_RETCODE_PARSE_ERROR; // todo: better error
    }
    else{
        ESP_LOGE(TAG, "No cleartext data provided to encrypt");
        return AES132_DEVICE_RETCODE_PARSE_ERROR; // todo: better error
    }

    cmd = AES132_OPCODE_ENCRYPT;
    mode = 0; // We don't care about the MAC, we want speed
    //mode = AES132_INCLUDE_SMALLZONE_SERIAL_COUNTER; // MAC Params
    param1 = key_id;
    param2 = len;

    mac_incr();

    res = aes132m_execute(cmd, mode, param1, param2,
            len, in, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( 0 == res ) {
        if( NULL != out_mac ) {
            memcpy(out_mac, &rx_buffer[AES132_RESPONSE_INDEX_DATA], 16);
        }

        if( NULL != out_data ) {
            memcpy(out_data, &rx_buffer[AES132_RESPONSE_INDEX_DATA+16],
                    ciphertext_len);
        }
        else {
            ESP_LOGE(TAG, "Cannot copy ciphertext to NULL pointer");
        }
    }
    else{
        nonce->valid = false;
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
        res = aes132_encrypt(data, data_len, AES132_KEY_ID_STRETCH, data, NULL);
        if( AES132_DEVICE_RETCODE_SUCCESS != res ) {
            ESP_LOGE(TAG, "Failed on iteration %d with retcode %02X\n", i, res);
            break;
        }
    }
    return res;
}

/* todo: Make this static */
uint8_t aes132_auth(uint8_t *key, uint16_t key_id) {
    /* Used to check a PIN attempt. Upon successful attempt, unlocks the 
     * corresponding UserZone 
     *
     * Requires a synchronized Nonce beforehand!*/
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = { 0 };
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = { 0 };
    uint8_t cmd, mode;
    uint16_t param1, param2;
    uint8_t *out_mac = &rx_buffer[AES132_RESPONSE_INDEX_DATA];

    cmd = AES132_OPCODE_AUTH;
    //mode = 0x03; // Mutual Authentication 
    mode = 0x02; // Outbound Authentication 
    param1 = key_id;
    //param2 = 0x0003; // R/W for UserZone Alowed
    param2 = 0x0000; // Ignored for outbound auth

    /* Assemble Mac-checking structure */
    struct aes132h_in_out mac_check_decrypt_param;
    mac_check_decrypt_param.opcode  = cmd;
    mac_check_decrypt_param.mode    = mode;
    mac_check_decrypt_param.param1  = param1;
    mac_check_decrypt_param.param2  = param2;
    mac_check_decrypt_param.key     = master_key;
    mac_check_decrypt_param.nonce   = get_nonce();
    mac_check_decrypt_param.in_mac = out_mac;
    mac_check_decrypt_param.in_data = NULL;
    mac_check_decrypt_param.out_data = NULL;

    res = aes132m_execute(cmd, mode, param1, param2,
              0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    //        len, in, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);

    /* Confirm response MAC */
    if( res ) {
        ESP_LOGE(TAG, "Auth command failed with return code 0x%02X", res);
    }
    else {
        ESP_LOGI(TAG, "outMAC "
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                out_mac[0], out_mac[1], out_mac[2], out_mac[3],
                out_mac[4], out_mac[5], out_mac[6], out_mac[7],
                out_mac[8], out_mac[9], out_mac[10], out_mac[11],
                out_mac[12], out_mac[13], out_mac[14], out_mac[15]);
    }

    // internally increments the mac_count
    //uint8_t device_count;
    //aes132_mac_count(&device_count);
    //ESP_LOGI(TAG, "Device mac_count: %d", device_count);

    res = aes132h_mac_check_decrypt(&mac_check_decrypt_param);

    if ( AES132_DEVICE_RETCODE_SUCCESS == res ) {
        ESP_LOGI(TAG, "AES132 returned valid MAC");
    }
    else {
        ESP_LOGE(TAG, "MAC verification error: %02X", res);
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
