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

uint8_t aes132_check_configlock(bool *status) {
    /* Returns true if device configuratoin is locked */
    uint8_t data;
    uint8_t res;
    res = aes132_blockread(&data, AES132_LOCK_CONFIG_ADDR, sizeof(data));
    if( !res && 0x55 == data ) {
        // Device configuration is unlocked
        *status = false;
    }
    else {
        *status = true;
    }
    return res;
}

uint8_t aes132_check_manufacturing_id(bool *status) {
    /* Returns true if device configuratoin is locked */
    uint8_t data[2];
    uint8_t res;
    res = aes132_blockread(data, AES132_MANUFACTURING_ID, sizeof(data));
    if( res ) {
    }
    else {
        ESP_LOGI(TAG, "Manufacturing ID: 0x%02X%02X", 
                data[0], data[1]);
    }
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

    /* Lock SmallZone */
    {
        ESP_LOGI(TAG, "Locking SmallZone");
        const uint8_t mode = mode_common | AES132_LOCK_SMALLZONE;
        uint16_t param1 = 0;
        // todo: compute checksum into param2
        uint16_t param2 = 0;
        res = aes132m_execute(cmd, mode, param1, param2,
                0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
        if( res ) {
            goto exit;
        }
    }

    /* Make Master UserZone Read-Only */
    {
        ESP_LOGI(TAG, "Locking UserZone 0 (master) read-only");
        const uint8_t mode = mode_common | AES132_LOCK_ZONECONFIG_RO;
        uint16_t param1 = 0;
        // todo: compute checksum into param2
        uint16_t param2 = 0;

        uint8_t data[16];
        res = aes132m_execute(cmd, mode, param1, param2,
                0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
        if( res ) {
            goto exit;
        }
    }

    /* Lock Configuration Memory */
    {
        ESP_LOGI(TAG, "Locking Configuration Memory");
        uint8_t mode = mode_common | AES132_LOCK_CONFIG;
        uint16_t param1 = 0;
        // todo: compute checksum into param2
        uint16_t param2 = 0;
        res = aes132m_execute(cmd, mode, param1, param2,
                0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
        if( res ) {
            goto exit;
        }
    }
    /* Lock Key Memory */
    {
        ESP_LOGI(TAG, "Locking Key Memory");
        const uint8_t mode = mode_common | AES132_LOCK_KEY;
        uint16_t param1 = 0;
        // todo: compute checksum into param2
        uint16_t param2 = 0;
        res = aes132m_execute(cmd, mode, param1, param2,
                0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
        if( res ) {
            goto exit;
        }
    }

exit:
    return res;
}

uint8_t aes132_blockread(uint8_t *data, const uint16_t address, const uint8_t count) {
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

/* Populates the output buffer with n_bytes of random numbers */
uint8_t aes132_rand(uint8_t *out, const size_t n_bytes) {
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

#if 0
/* todo: this should take in authorizing key, mac check info, etc */
uint8_t aes132_key_load(uint128_t key, const uint8_t key_id) {
    /* We use this to load in the PIN authorization key */

    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};
    uint8_t cmd, mode;
    uint16_t param1, param2;
    uint8_t in_mac[16];
    uint8_t enc_key[16];

    /* Configuration - Page 51
     * Data1 - 16 Bytes - Integrity MAC for the input data
     * Data2 - 16 Bytes - Encrypted Key Value
     *
     */
    cmd = AES132_OPCODE_KEY_LOAD;
    mode = 0x01;
    mode |= AES132_INCLUDE_SMALLZONE_SERIAL_COUNTER; // MAC Params
    param1 = key_id;
    param2 = 0x0000; // only used for VolatileKey
    {
        // A lot of this stuff is on page 109
        mbedtls_aes_context aes;
        mbedtls_aes_init( &aes );
        mbedtls_aes_setkey_enc( &aes, master_key, 128 );

        uint128_t iv = { 0 }; // ataes132a uses an all zero IV
        uint128_t in = { 0 };
        uint128_t out = { 0 };
        uint128_t cleartext_mac = { 0 };
        const uint16_t auth_data_len = 14;

        mac_incr();

        /* CBC Section */
        mac_create_b0(in, 16); // 16 bytes of encrypted key payload
        ESP_ERROR_CHECK(mbedtls_aes_crypt_cbc( &aes, 
                    MBEDTLS_AES_ENCRYPT, 16, iv, in, out )); 
        ESP_LOGI(TAG, "IV "
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                iv[0], iv[1], iv[2], iv[3], iv[4], iv[5], iv[6], iv[7],
                iv[8], iv[9], iv[10], iv[11], iv[12], iv[13], iv[14],
                iv[15]);
        ESP_LOGI(TAG, "out "
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7],
                out[8], out[9], out[10], out[11], out[12], out[13], out[14],
                out[15]);

        create_auth_only_block1( &in[2], cmd, mode, param1, param2, auth_data_len, true );
        ESP_ERROR_CHECK(mbedtls_aes_crypt_cbc( &aes, 
                    MBEDTLS_AES_ENCRYPT, 16, iv, in, out ));
        ESP_ERROR_CHECK(mbedtls_aes_crypt_cbc( &aes, 
                    MBEDTLS_AES_ENCRYPT, 16, iv, key, cleartext_mac ));

        /* CTR Section */
        /* We are calling ECB functions because we basically implement our own
         * custom CTR scheme */
        uint16_t ctr = 0;
        //mac_create_a(in, ctr++);
        ESP_ERROR_CHECK(mbedtls_aes_crypt_ecb( &aes, 
                    MBEDTLS_AES_ENCRYPT, in, in_mac));
        xor128(in_mac, cleartext_mac);
        mac_create_a(in, ctr++);
        ESP_ERROR_CHECK(mbedtls_aes_crypt_ecb( &aes,
                    MBEDTLS_AES_ENCRYPT, in, enc_key));
        xor128(enc_key, key);

        /* mbedtls aes context cleanup */
        mbedtls_aes_free( &aes );
    }
    res = aes132m_execute(cmd, mode, param1, param2,
            sizeof(in_mac), in_mac,
            sizeof(enc_key), enc_key,
            0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( res ) {
        ESP_LOGE(TAG, "Key Load Failed! Dumping...");
        ESP_LOGE(TAG, "mode: %02X", mode);
        ESP_LOGE(TAG, "param1: %04X", param1);
        ESP_LOGE(TAG, "param2: %04X", param2);
    }

    return res;
}
#endif

static uint8_t lin2bin(uint8_t bin) {
    /* Used in Counter command; counts number of zero bits from lsb */
    uint8_t count = 0;
    while( !(bin & 0x1) ) {
        count++;
        bin >>= 1;
    }
    return count;
}

uint8_t aes132_counter(uint32_t *count, uint8_t counter_id) {
    /* MVP
     * Todo:
     *     * error handling
     *     * robust unit tests
     */
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};
    uint8_t cmd, mode;
    uint16_t param1, param2;

    cmd = AES132_OPCODE_COUNTER;
    mode = 0x01; // Read the Counter
    param1 = counter_id;
    param2 = 0x0000;
    res = aes132m_execute(cmd, mode, param1, param2,
            0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);

    // todo: error handling
    // todo: mac checking

    // Interpretting the counter
    uint8_t lin_count, count_flag;
    uint16_t bin_count;
    lin_count = rx_buffer[AES132_RESPONSE_INDEX_DATA];
    count_flag = rx_buffer[AES132_RESPONSE_INDEX_DATA+1];
    bin_count = rx_buffer[AES132_RESPONSE_INDEX_DATA+2]<<8 \
                | rx_buffer[AES132_RESPONSE_INDEX_DATA+3];
    *count = (bin_count*32) + (count_flag/2)*8 + lin2bin(lin_count);
    return 0; // success
}

#if 0
uint8_t aes132_key_create(uint8_t key_id) {
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = { 0 };
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = { 0 };
    uint8_t cmd, mode;
    uint16_t param1, param2;

    mac_incr();
    // todo: mac stuff
    cmd = AES132_OPCODE_KEY_CREATE;
    mode = 0x07; 
    mode |= AES132_INCLUDE_SMALLZONE_SERIAL_COUNTER; // MAC Params
    param1 = key_id;
    param2 = 0x0000;
    res = aes132m_execute(cmd, mode, param1, param2,
            0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( res ) {
        if( AES132_FUNCTION_RETCODE_BAD_CRC_TX == res ) {
            mac_count--; // mac_count doesn't increment on bad crc
        }
        if( I2C_FUNCTION_RETCODE_NACK == res ) {
            ESP_LOGE(TAG, "KeyCreate Nack!");
        }
        printf("KeyCreate TX Buffer: \n");
        for(uint8_t i=0; i<sizeof(tx_buffer); i++) {
            printf("%02X ", tx_buffer[i]);
        }
        printf("\n");

        printf("KeyCreate RX Buffer: \n");
        for(uint8_t i=0; i<sizeof(rx_buffer); i++) {
            printf("%02X ", rx_buffer[i]);
        }
        printf("\n");
    }
    return res;
}
#endif

#if 0
uint8_t aes132_encrypt(const uint8_t *in, uint8_t len, uint8_t key_id,
        uint8_t *out_data, uint8_t *out_mac) {
    /* Encrypts upto 32 bytes of data (*in with length len) using key_id
     * returns cipher text (*out)
     * len must be <=32.
     * out_data must be able to handle 16 bytes (<=16byte in) or 
     * 32 bytes (<=32 byte in)
     * todo: something with the output MAC*/
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = { 0 };
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = { 0 };
    uint8_t cmd, mode;
    uint16_t param1, param2;

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

    mac_incr(); // Encrypt command returns a MAC, increase local mac_count

    cmd = AES132_OPCODE_ENCRYPT;
    mode = 0; // We don't care about the MAC, we want speed
    //mode = AES132_INCLUDE_SMALLZONE_SERIAL_COUNTER; // MAC Params
    param1 = key_id;
    param2 = len;
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
    else {
        if( AES132_FUNCTION_RETCODE_BAD_CRC_TX == res ) {
            mac_count--; // mac_count doesn't increment on bad crc
        }
    }
    return res;
}
#endif
#if 0
uint8_t aes132_auth(uint16_t key_id) {
    /* Used to check a PIN attempt. Upon successful attempt, unlocks the 
     * corresponding UserZone 
     *
     * Requires a synchronized Nonce beforehand!*/
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = { 0 };
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = { 0 };
    uint8_t cmd, mode;
    uint16_t param1, param2;
    uint128_t out;

    cmd = AES132_OPCODE_AUTH;
    //mode = 0x03; // Mutual Authentication 
    mode = 0x02; // Outbound Authentication 
    mode |= AES132_INCLUDE_SMALLZONE_SERIAL_COUNTER; // MAC Params
    param1 = key_id;
    param2 = 0x0003; // R/W for UserZone Alowed

    res = aes132m_execute(cmd, mode, param1, param2,
              0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    //        len, in, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);

    /* Confirm response MAC */
    if( res ) {
        ESP_LOGE(TAG, "Auth command failed with return code 0x%02X", res);
    }
    else {
        memcpy(out, &rx_buffer[AES132_RESPONSE_INDEX_DATA], sizeof(out));
        ESP_LOGI(TAG, "outMAC "
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7],
                out[8], out[9], out[10], out[11], out[12], out[13], out[14],
                out[15]);
    }
    {
        // Attempting to compute the same outmac
        // A lot of this stuff is on page 109
        mbedtls_aes_context aes;
        mbedtls_aes_init( &aes );
        mbedtls_aes_setkey_enc( &aes, master_key, 128 );

        uint128_t iv = { 0 }; // ataes132a uses an all zero IV
        uint128_t in = { 0 };
        uint128_t out = { 0 };
        uint128_t cleartext_mac = { 0 };
        const uint16_t auth_data_len = 11; // pretty sure this is 11

        mac_incr();

        /* CBC Section */
        // b0
        mac_create_b0(in, 0); // auth only
        ESP_ERROR_CHECK(mbedtls_aes_crypt_cbc( &aes, 
                    MBEDTLS_AES_ENCRYPT, 16, iv, in, out )); 
        ESP_LOGI(TAG, "in "
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7],
                in[8], in[9], in[10], in[11], in[12], in[13], in[14],
                in[15]);
        ESP_LOGI(TAG, "out "
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7],
                out[8], out[9], out[10], out[11], out[12], out[13], out[14],
                out[15]);

        // b1
        create_auth_only_block1( in, cmd, mode, param1, param2, 
                auth_data_len, false);
        ESP_ERROR_CHECK(mbedtls_aes_crypt_cbc( &aes, 
                    MBEDTLS_AES_ENCRYPT, 16, iv, in, cleartext_mac ));
        ESP_LOGI(TAG, "computed cleartext out_mac "
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                cleartext_mac[0], cleartext_mac[1], cleartext_mac[2], cleartext_mac[3], cleartext_mac[4], cleartext_mac[5], cleartext_mac[6], cleartext_mac[7],
                cleartext_mac[8], cleartext_mac[9], cleartext_mac[10], cleartext_mac[11], cleartext_mac[12], cleartext_mac[13], cleartext_mac[14],
                cleartext_mac[15]);

        /* CTR Section */
        /* We are calling ECB functions because we basically implement our own
         * custom CTR scheme */
        uint16_t ctr = 0;
        mac_create_a(in, ctr++);
        uint128_t in_mac;
        ESP_ERROR_CHECK(mbedtls_aes_crypt_ecb( &aes, 
                    MBEDTLS_AES_ENCRYPT, in, in_mac));
        ESP_LOGI(TAG, "pre xor out_mac "
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                in_mac[0], in_mac[1], in_mac[2], in_mac[3], in_mac[4], in_mac[5], in_mac[6], in_mac[7],
                in_mac[8], in_mac[9], in_mac[10], in_mac[11], in_mac[12], in_mac[13], in_mac[14],
                in_mac[15]);

        xor128(in_mac, cleartext_mac);

        ESP_LOGI(TAG, "computed encrypted out_mac "
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                in_mac[0], in_mac[1], in_mac[2], in_mac[3], in_mac[4], in_mac[5], in_mac[6], in_mac[7],
                in_mac[8], in_mac[9], in_mac[10], in_mac[11], in_mac[12], in_mac[13], in_mac[14],
                in_mac[15]);

        /* mbedtls aes context cleanup */
        mbedtls_aes_free( &aes );
    }
    {
        mbedtls_ccm_context ccm;
        mbedtls_ccm_init( &ccm );
        mbedtls_ccm_setkey( &ccm, MBEDTLS_CIPHER_ID_AES, master_key, 128 );

        uint8_t iv[13] = { 0 }; // ataes132a uses an all zero IV
        memcpy(iv, nonce, 12);
        iv[12] = mac_count;
        uint128_t in = { 0 };
        uint128_t out = { 0 };
        uint128_t tag = { 0 };

        create_auth_only_block1( in, cmd, mode, param1, param2, 
                7, false);

        ESP_ERROR_CHECK( mbedtls_ccm_encrypt_and_tag( &ccm, 16,
                    iv, 13, NULL, 0, 
                    in, out, tag, 16 ));
        ESP_LOGI(TAG, "computed tag "
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                tag[0], tag[1], tag[2], tag[3], tag[4], tag[5], tag[6], tag[7],
                tag[8], tag[9], tag[10], tag[11], tag[12], tag[13], tag[14],
                tag[15]);

        mbedtls_ccm_free( &ccm );
    }

    return res;
}
#endif

