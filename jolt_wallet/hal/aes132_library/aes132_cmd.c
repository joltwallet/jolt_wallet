/* Wrapping opcodes for easier use */
#include <stdint.h>
#include <string.h>
#include "aes132_mac.h"
#include "aes132_comm_marshaling.h"
#include "aes132_i2c.h" // For ReturnCode macros
#include "aes132_cmd.h"
#include "i2c_phys.h" // For res macros
#include "esp_log.h"
#include "sodium.h"
#include "jolttypes.h"

static const char TAG[] = "aes132_cmd";

static uint8_t lin2bin(uint8_t bin);

static uint8_t lin2bin(uint8_t bin) {
    /* Used in Counter command; counts number of zero bits from lsb */
    uint8_t count = 0;
    while( !(bin & 0x1) ) {
        count++;
        bin >>= 1;
    }
    return count;
}

uint8_t aes132_auth(const uint8_t *key, uint16_t key_id,
        struct aes132h_nonce_s *nonce) {
    /* Used to check a PIN attempt. Upon successful attempt, unlocks the 
     * corresponding UserZone. 
     */
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = { 0 };
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = { 0 };
    uint8_t cmd, mode;
    uint16_t param1, param2;
    uint8_t in_mac[16];
    uint8_t *out_mac = &rx_buffer[AES132_RESPONSE_INDEX_DATA];

    if( 254 == nonce->value[12] || nonce->valid==false ) {
        res = aes132_nonce(nonce, NULL);
        if ( res ) {
            ESP_LOGE(TAG, "Error Refreshing nonce for mutual auth");
        }
    }

    cmd = AES132_OPCODE_AUTH;
    mode = 0x03; // Mutual Authentication 
    param1 = key_id;
    param2 = 0x0003; // R/W for UserZone Alowed

    /* Assemble and compute Inbound MAC */
    ESP_LOGI(TAG, "Computing Inbound MAC");
    //aes132_mac_incr(nonce);
    struct aes132h_in_out mac_compute_encrypt_param;
    mac_compute_encrypt_param.opcode  = cmd;
    mac_compute_encrypt_param.mode    = mode;
    mac_compute_encrypt_param.param1  = param1;
    mac_compute_encrypt_param.param2  = param2;
    mac_compute_encrypt_param.key     = key;
    mac_compute_encrypt_param.nonce   = nonce;
    mac_compute_encrypt_param.out_mac = in_mac;
    res = aes132h_mac_compute_encrypt(&mac_compute_encrypt_param);
    if ( AES132_DEVICE_RETCODE_SUCCESS != res ) {
        ESP_LOGE(TAG, "Error computing Inbound MAC: 0x%02X", res);
        goto exit;
    }

    /* Assemble Outbound MAC struct */
    ESP_LOGI(TAG, "Checking Outbound MAC");
    struct aes132h_in_out mac_check_decrypt_param;
    mac_check_decrypt_param.opcode  = cmd;
    mac_check_decrypt_param.mode    = mode;
    mac_check_decrypt_param.param1  = param1;
    mac_check_decrypt_param.param2  = param2;
    mac_check_decrypt_param.key     = key;
    mac_check_decrypt_param.nonce   = nonce;
    mac_check_decrypt_param.in_mac = out_mac;

    res = aes132m_execute(cmd, mode, param1, param2,
            sizeof(in_mac), in_mac, 0, NULL, 0, NULL, 0, NULL,
            tx_buffer, rx_buffer);
    if ( AES132_DEVICE_RETCODE_SUCCESS != res ) {
        ESP_LOGE(TAG, "Auth command failed with return code 0x%02X", res);
        nonce->valid = false;
        goto exit;
    }

    /* Check OutBound MAC */
    res = aes132h_mac_check_decrypt(&mac_check_decrypt_param);
    if ( AES132_DEVICE_RETCODE_SUCCESS == res ) {
        ESP_LOGI(TAG, "AES132 returned valid MAC");
    }
    else {
        ESP_LOGE(TAG, "AES132 returned invalid MAC");
        nonce->valid = false;
        goto exit;
    }
exit:
    return res;
}

uint8_t aes132_blockread(uint8_t *data, const uint16_t address,
        const uint8_t count) {
    /* BlockRead Command reads 1~32 bytes of plaintext from userzone or 
     * configuration memory. Standard eeprom read commands can also read userzone
     * data if authentication nor encryption is required. Standard eeprom read
     * commands cannot read configuration memory.
     *
     * Requested data cannot cross page boundaries.
     *
     * Configuration memory can only be read via the blockread command.
     */
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};
    uint8_t cmd, mode;
    uint16_t param1, param2;

    cmd = AES132_OPCODE_BLOCK_READ;
    mode = 0x00; // Must be zero
    param1 = address;
    param2 = count;
    res = aes132m_execute(cmd, mode, param1, param2,
            0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( !res ) {
        memcpy(data, &rx_buffer[AES132_RESPONSE_INDEX_DATA], count);
    }
    return res;
}

uint8_t aes132_check_configlock(bool *status) {
    /* Returns true if device configuratoin is locked */
    uint8_t data = 0x55;
    uint8_t res;
    res = aes132_blockread(&data, AES132_LOCK_CONFIG_ADDR, sizeof(data));
    if( 0x55 == data ) {
        // Device configuration is unlocked
        *status = false;
    }
    else {
        *status = true;
    }
    return res;
}

uint8_t aes132_counter(const uint8_t *mac_key, uint32_t *count,
        uint8_t counter_id, struct aes132h_nonce_s *nonce) {
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};
    uint8_t cmd, mode;
    uint16_t param1, param2;
    uint8_t *count_value = &rx_buffer[AES132_RESPONSE_INDEX_DATA];
    uint8_t *out_mac = &rx_buffer[AES132_RESPONSE_INDEX_DATA+4];

    cmd = AES132_OPCODE_COUNTER;
    mode = 0x03; // Read the Counter with MAC
    param1 = counter_id;
    param2 = 0x0000;

    if(nonce->valid==false) {
        res = aes132_nonce(nonce, NULL);
        if ( res ) {
            ESP_LOGE(TAG, "Error Refreshing nonce for mutual auth");
        }
    }

    /* Assemble Outbound MAC struct */
    struct aes132h_in_out mac_check_decrypt_param;
    mac_check_decrypt_param.opcode  = cmd;
    mac_check_decrypt_param.mode    = mode;
    mac_check_decrypt_param.param1  = param1;
    mac_check_decrypt_param.param2  = param2;
    mac_check_decrypt_param.key     = mac_key;
    mac_check_decrypt_param.nonce   = nonce;
    mac_check_decrypt_param.in_mac = out_mac;
    mac_check_decrypt_param.count_value = count_value;

    res = aes132m_execute(cmd, mode, param1, param2,
            0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( res ) {
        ESP_LOGE(TAG, "Error executing Counter command. Return Code 0x%02X",
                res);
        nonce->valid = false;
        goto exit;
    }

    res = aes132h_mac_check_decrypt(&mac_check_decrypt_param);
    if ( AES132_DEVICE_RETCODE_SUCCESS == res ) {
        ESP_LOGI(TAG, "Successfully verified MAC Counter Read on Key Slot %d",
                counter_id);
    }
    else {
        ESP_LOGE(TAG, "AES132 returned invalid MAC");
        nonce->valid = false;
        goto exit;
    }

    /* Interpret Counter */
    uint8_t lin_count, count_flag;
    uint16_t bin_count;
    lin_count = count_value[0];
    count_flag = count_value[1];
    bin_count = ((uint16_t)count_value[2])<<8 | count_value[3];
    *count = (bin_count*32) + (count_flag/2)*8 + lin2bin(lin_count);
exit:
    return res;
}

uint8_t aes132_encrypt(const uint8_t *in, uint8_t len, uint8_t key_id,
        uint8_t *out_data, uint8_t *out_mac, struct aes132h_nonce_s *nonce) {
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

    cmd = AES132_OPCODE_ENCRYPT;
    mode = 0; // We don't care about the MAC, we want speed
    param1 = key_id;
    param2 = len;

    aes132_mac_incr(nonce);

    res = aes132m_execute(cmd, mode, param1, param2,
            len, in, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( 0 == res ) {
        if( NULL != out_mac ) {
            memcpy(out_mac, &rx_buffer[AES132_RESPONSE_INDEX_DATA], 16);
        }
        if( NULL != out_data ) {
            memcpy(out_data, &rx_buffer[AES132_RESPONSE_INDEX_DATA+16],
                    len);
        }
        else {
            ESP_LOGE(TAG, "Cannot copy ciphertext to NULL pointer");
        }
    }
    else {
        nonce->valid = false;
    }
    return res;
}

uint8_t aes132_key_create(uint8_t key_id, struct aes132h_nonce_s *nonce) {
    /* Only used to create the stretch key */
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = { 0 };
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = { 0 };
    uint8_t cmd, mode;
    uint16_t param1, param2;

    // Increment MacCount before operation. 
    aes132_mac_incr(nonce);

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

uint8_t aes132_key_load(const uint128_t parent_key, uint128_t child_key,
        const uint8_t key_id, struct aes132h_nonce_s *nonce) {
    /* We use this to load in the PIN authorization key 
     * Encrypts child_key with parent_key, and sends it over*/

    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};
    uint8_t cmd, mode;
    uint16_t param1, param2;
    uint8_t in_mac[16];
    uint8_t enc_key[16];

    if(nonce->valid==false) {
        res = aes132_nonce(nonce, NULL);
        if ( res ) {
            ESP_LOGE(TAG, "Error Refreshing nonce for mutual auth");
        }
    }

    /* Configuration - Page 51
     * Data1 - 16 Bytes - Integrity MAC for the input data
     * Data2 - 16 Bytes - Encrypted Key Value
     */
    cmd = AES132_OPCODE_KEY_LOAD;
    mode = 0x01;
    param1 = key_id;
    param2 = 0x0000; // only used for VolatileKey

    /* Assemble and compute Inbound MAC */
    struct aes132h_in_out mac_compute_encrypt_param;
    mac_compute_encrypt_param.opcode   = cmd;
    mac_compute_encrypt_param.mode     = mode;
    mac_compute_encrypt_param.param1   = param1;
    mac_compute_encrypt_param.param2   = param2;
    mac_compute_encrypt_param.key      = parent_key;
    mac_compute_encrypt_param.nonce    = nonce;
    mac_compute_encrypt_param.out_mac  = in_mac;
    mac_compute_encrypt_param.in_data  = child_key;
    mac_compute_encrypt_param.out_data = enc_key;
    res = aes132h_mac_compute_encrypt(&mac_compute_encrypt_param);
    if ( AES132_DEVICE_RETCODE_SUCCESS != res ) {
        ESP_LOGE(TAG, "Error computing Inbound MAC: 0x%02X", res);
        goto exit;
    }

    res = aes132m_execute(cmd, mode, param1, param2,
            sizeof(in_mac), in_mac,
            sizeof(enc_key), enc_key,
            0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( res ) {
        ESP_LOGE(TAG, "Key Load failed with error code 0x%02X", res);
        nonce->valid = false;
        goto exit;
    }

exit:
    return res;
}

uint8_t aes132_legacy(const uint8_t key_id, uint8_t *data) {
    /* Only used for key stretching.
     * Passes 16-bytes of data through the AES engine using key at key_id.
     * Results are returned in data. */
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    CONFIDENTIAL uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};
    uint8_t cmd, mode;
    uint16_t param1, param2;

    cmd = AES132_OPCODE_LEGACY;
    mode = 0x00; // Must be 0x00
    param1 = key_id;
    param2 = 0x0000; // Must be 0x0000
    res = aes132m_execute(cmd, mode, param1, param2,
            16, data, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( res ) {
        ESP_LOGE(TAG, "Error issuing Legacy cmd: 0x%02X", res);
        goto exit;
    }
    memcpy(data, &rx_buffer[AES132_RESPONSE_INDEX_DATA], 16);
exit:
    sodium_memzero(rx_buffer, sizeof(rx_buffer));
    return res;
}

uint8_t aes132_lock_device() {
    /* Locks smallzone, config memory, key memory, and makes the master
     * UserZone read-only 
     *
     * Warning; this function has permament physical impacts.
     * */
    // Configuration Memory MUST be locked before Key Memory
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};

    const uint8_t cmd = AES132_OPCODE_LOCK;
    // Require Validate memory checksum in Param2
    const uint8_t mode_common = 0x04;
    uint8_t mode;
    uint16_t param1, param2;

    /* Lock SmallZone */
    ESP_LOGI(TAG, "Locking SmallZone");
    mode = mode_common | AES132_LOCK_SMALLZONE;
    param1 = 0;
    // todo: compute checksum into param2
    param2 = 0;
    res = aes132m_execute(cmd, mode, param1, param2,
            0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( res ) {
        goto exit;
    }

    /* Make Master UserZone Read-Only */
    ESP_LOGI(TAG, "Locking UserZone 0 (master) read-only");
    mode = mode_common | AES132_LOCK_ZONECONFIG_RO;
    param1 = 0;
    // todo: compute checksum into param2
    param2 = 0;

    uint8_t data[16];
    res = aes132m_execute(cmd, mode, param1, param2,
            0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( res ) {
        goto exit;
    }

    /* Lock Configuration Memory */
    ESP_LOGI(TAG, "Locking Configuration Memory");
    mode = mode_common | AES132_LOCK_CONFIG;
    param1 = 0;
    // todo: compute checksum into param2
    param2 = 0;
    res = aes132m_execute(cmd, mode, param1, param2,
            0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( res ) {
        goto exit;
    }

    /* Lock Key Memory */
    ESP_LOGI(TAG, "Locking Key Memory");
    mode = mode_common | AES132_LOCK_KEY;
    param1 = 0;
    // todo: compute checksum into param2
    param2 = 0;
    res = aes132m_execute(cmd, mode, param1, param2,
            0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( res ) {
        goto exit;
    }

exit:
    return res;
}

uint8_t aes132_mac_count(uint8_t *count) {
    /* Gets the maccount */
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};
    uint8_t cmd, mode;
    uint16_t param1, param2;

    cmd = AES132_OPCODE_INFO;
    mode = 0x00;
    param1 = 0x0000;
    param2 = 0x0000;
    res = aes132m_execute(cmd, mode, param1, param2,
        0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( AES132_FUNCTION_RETCODE_SUCCESS == res ) {
        *count = rx_buffer[AES132_RESPONSE_INDEX_DATA + 1];
    }
    else {
    }
    return res;
}

#define MAC_COUNT_LOCKSTEP_CHECK false
uint8_t aes132_mac_incr(struct aes132h_nonce_s *nonce) {
    /* Used to increase MAC Counter when we don't actually care about
     * the produced MAC */
    uint8_t *mac_count;
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
    if( UINT8_MAX == *mac_count ) {
        ESP_LOGD(TAG, "local mac_count maxed out, issuing rand Nonce");
        nonce->valid = false;
        uint8_t res;
        res = aes132_nonce(nonce, NULL); 
        if( res ) {
            ESP_LOGE(TAG, "Error %02X generating new nonce", res);
            esp_restart();
        }
        else {
            ESP_LOGI(TAG, "Nonce Refreshed");
        }
    }
    (*mac_count)++;
    return *mac_count;
}


uint8_t aes132_nonce(struct aes132h_nonce_s *nonce, const uint8_t *in_seed) {
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
    mode |= AES132_EEPROM_RNG_UPDATE;
    param1 = 0x0000; // Always 0
    param2 = 0x0000; // Always 0
    res = aes132m_execute(cmd, mode, param1, param2,
            12, data, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( res ) {
        /* Error */
        ESP_LOGE(TAG, "Nonce returncode: %02X",
                rx_buffer[AES132_RESPONSE_INDEX_RETURN_CODE]);
        nonce->valid = false;
    }
    else {
        /* Success */
        struct aes132h_nonce_in_out nonce_param;
        nonce_param.mode    = mode;
        nonce_param.in_seed = data;
        nonce_param.random  = &rx_buffer[AES132_RESPONSE_INDEX_DATA];
        nonce_param.nonce   = nonce;
        res = aes132h_nonce(&nonce_param);
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
        ESP_LOGI(TAG, "Setting Nonce MacCount to 0");
        nonce->value[12] = 0;
        ESP_LOGI(TAG, "Setting Nonce Valid Flag True");
        nonce->valid = true;
    }
    return res;
}

uint8_t aes132_rand(uint8_t *out, const size_t n_bytes) {
    /* Populates the output buffer with n_bytes of random numbers.
     * Sends Rand cmds to device as many times as necessary */
    uint8_t res;
    uint8_t cmd, mode;
    uint16_t param1, param2;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    CONFIDENTIAL uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};

    cmd = AES132_OPCODE_RANDOM;
    mode = 0x02;
    param1 = 0x0000;
    param2 = 0x0000;

    size_t out_head = 0; // Current index into out buffer
    size_t n_copy = 0; // number of bits top copy from rand buffer
    do {
        res = aes132m_execute(cmd, mode, param1, param2,
            0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
        if( AES132_FUNCTION_RETCODE_SUCCESS != res ) {
            goto exit;
        }
        if( res ) {
            return res;
        }

        // Copy over up to 16 bytes
        if( n_bytes - out_head > 16 ) {
            n_copy = 16;
        }
        else {
            n_copy = n_bytes - out_head;
        }
        memcpy(&out[out_head], &rx_buffer[AES132_RESPONSE_INDEX_DATA], n_copy);
        out_head += n_copy;
    } while( n_copy > 0 );

exit:
    sodium_memzero(rx_buffer, sizeof(rx_buffer));
    return res; // success
}
