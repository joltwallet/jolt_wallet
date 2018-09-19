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
#include "sodium.h"

static const char TAG[] = "aes132_jolt";
static uint8_t *master_key = NULL;
static const uint16_t manufacturing_id = 0x00EE; // default manufacturing id
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
static uint8_t mac_count = 0;
static uint8_t nonce[12] = { 0 };

static uint8_t mac_incr();
static uint8_t aes132_nonce(const uint8_t *in_seed);

#define MAC_COUNT_LOCKSTEP_CHECK true
static uint8_t mac_incr() {
#if MAC_COUNT_LOCKSTEP_CHECK
    {
        // Read the aes132m mac_count to make sure we are in lock-step
        uint8_t device_count;
        aes132_mac_count(&device_count);
        if( device_count != mac_count ) {
            ESP_LOGE(TAG, "mac_count desyncronized before incrementing.\n"
                    "Device=%d; Local=%d",
                    device_count, mac_count);
        }
    }
#endif
    if( UINT8_MAX == mac_count ) {
        // todo: error handling, nonce sync?
        ESP_LOGD(TAG, "local mac_count maxed out, issuing rand Nonce");
        uint8_t res = aes132_nonce(NULL);
        mac_count = 0;
    }
    mac_count++;
    return mac_count;
}

static uint8_t aes132_nonce(const uint8_t *in_seed) {
    /* Random is only populated if in_seed is NULL.
     *
     * in_seed should be 12 bytes.
     *
     * */
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};

    uint8_t cmd, mode;
    uint16_t param1, param2;
    uint8_t data[12] = { 0 };

    cmd = AES132_OPCODE_NONCE;
    if( NULL == in_seed) {
        mode = 0x01; // Use the inSeed as Nonce
    }
    else {
        mode = 0x00; // Generate a random Nonce using internal RNG
    }
    //mode |= AES132_EEPROM_RNG_UPDATE;
    param1 = 0x0000; // Always 0
    param2 = 0x0000; // Always 0
    if( NULL != in_seed ) {
        memcpy(data, in_seed, sizeof(data));
    }
    res = aes132m_execute(cmd, mode, param1, param2,
            12, data, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( res ) {
        ESP_LOGE(TAG, "Nonce returncode: %02X",
                rx_buffer[AES132_RESPONSE_INDEX_RETURN_CODE]);
    }
    else {
        mac_count = 0; // or is this reset no matter what?
        memcpy(nonce, &rx_buffer[AES132_RESPONSE_INDEX_DATA], 12);
        ESP_LOGI(TAG, "Local Nonce updated to %02X %02X %02X %02X %02X "
                "%02X %02X %02X %02X %02X %02X %02X",
                nonce[0], nonce[1], nonce[2], nonce[3],
                nonce[4], nonce[5], nonce[6], nonce[7],
                nonce[8], nonce[9], nonce[10], nonce[11] );
    }
    return res;
}


uint8_t aes132_load_master_key() {
    /* Allocates space for the master key on the heap 
     * If device is unlocked:
     *     * Generate Master Key from ESP32 Entropy
     *     * Configure Device
     *     * Encrypt and Backup Master Key to MasterUserZone
     *     * Write key to slot 0
     *     * Lock Device
     * If device is locked:
     *     * Load and decrypt Master key from MasterUserZone
     */
    uint8_t res;
    if( NULL==master_key ) {
        master_key = sodium_malloc(16);
    }
    if( NULL==master_key ){
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
            ((uint32_t*)master_key)[i] = 0x1111111111;
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
        ESP_LOGI(TAG, "Writing Master Key to Key0");
        res = aes132m_write_memory(16,
                AES132_KEY_CONFIG_ADDR(AES132_KEY_ID_MASTER), master_key);
        ESP_LOGI(TAG, "Write key0 result: %d", res);
        
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
    return 0;
}


