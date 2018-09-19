#include <string.h>                    // needed for memcpy()
#include "aes132_comm_marshaling.h"    // definitions and declarations for the Command Marshaling module
#include "aes132_i2c.h" // For ReturnCode macros
#include "aes132_conf.h"
#include "esp_log.h"
#include "sodium.h"


/* Defines configuration and justification for Jolt */
static const char TAG[] = "aes132_con";


uint8_t aes132_write_chipconfig() {
    uint8_t res;
    uint8_t config = 
            AES132_CHIP_CONFIG_ENC_DEC_EN | // Used for Key Stretching
            AES132_CHIP_CONFIG_POWER_ACTIVE;
    res = aes132m_write_memory(sizeof(config),
            AES132_CHIPCONFIG_ADDR, &config);
    return res;
}

uint8_t aes132_write_counterconfig() {
    /* Only imposes restrictions on the usage of the Counter Command. 
     * Does not influence which keys use which counters. */
    uint8_t res;
    uint8_t config[2] = { 0 };
    /* No bits set; 
     *     * we don't need the increment option.
     *     * We use key_id 0 for MACs.
     */
    /* Write the same CounterConfig to all 16 registers */
    for(uint8_t i=0; i<16; i++) {
        res = aes132m_write_memory(sizeof(config),
                AES132_COUNTER_CONFIG_ADDR(i),
                &config);
        if( res ) {
            ESP_LOGW(TAG, "Wrote up to %d CounterConfig. "
                    "Error 0x%02x on CounterConfig %d", i, res, i);
            return res;
        }
    }
    return res;
}

uint8_t aes132_write_keyconfig() {
    uint8_t config_master[4] = { 0 };
    config_master[0] = 
        //AES132_KEY_CONFIG_EXTERNAL_CRYPTO | // Prohibit Encrypt/Decrypt
        //AES132_KEY_CONFIG_INBOUND_AUTH | // Can be used for other purposes
        AES132_KEY_CONFIG_RANDOM_NONCE | // Prevent Spoofing of Nonces 
        //AES132_KEY_CONFIG_LEGACY_OK | // Never allow dangerous Legacy cmd
        //AES132_KEY_CONFIG_AUTH_KEY | // Prior authentication not required 
        //AES132_KEY_CONFIG_CHILD | // Prohibit update via KeyCreate/KeyLoad 
        //AES132_KEY_CONFIG_PARENT | // VolatileKey isn't used  
        //AES132_KEY_CONFIG_CHANGE_KEYS | // Cannot be overwrited via EncWrite
        0;
    config_master[1] =
        //AES132_KEY_CONFIG_COUNTER_LIMIT | // No usage limit
        //AES132_KEY_CONFIG_CHILD_MAC | // Irrelevant; KeyCreate can't target
        //AES132_KEY_CONFIG_AUTH_OUT | // I2C auth signaling disabled
        //AES132_KEY_CONFIG_AUTH_OUT_HOLD | // Irrelevant; Auth signaling disabled
        //AES132_KEY_CONFIG_IMPORT_OK | // Prohibit KeyImport
        //AES132_KEY_CONFIG_EXPORT_AUTH | // Irreleant; KeyCreate can't target
        //AES132_KEY_CONFIG_TRANSFER_OK | // Prohibit KeyTransfer
        //AES132_KEY_CONFIG_AUTH_COMPUTE | // Key cannot be used for auth_compute
        0;
    config_master[2] = 
        AES132_KEY_CONFIG_COUNTER_NUM(0) | // Not used
        AES132_KEY_CONFIG_LINK_POINTER(0) ; // Not used
    config_master[3] = 
        //AES_132_KEY_CONFIG_DEC_READ | // DecRead and WriteCompute prohibited
        0;
    res = aes132m_write_memory(sizeof(config_master),
            AES132_KEY_CONFIG_ADDR(AES132_KEY_ID_MASTER), &config_master);
    if( res ) {
        ESP_LOGE(TAG, "Failed writing Master KeyConfig "
                "Error Return Code 0x%02X",
                res);
        return res;
    }
    

    /* Stretch is only used for Key streshing via Encrypt command */
    uint8_t config_stretch[4] = { 0 };
    config_stretch[0] = 
        AES132_KEY_CONFIG_EXTERNAL_CRYPTO | // Allow Encrypt/Decrypt
        //AES132_KEY_CONFIG_INBOUND_AUTH | // Can be used for other purposes
        AES132_KEY_CONFIG_RANDOM_NONCE | // Prevent Spoofing of Nonces 
        //AES132_KEY_CONFIG_LEGACY_OK | // Never allow dangerous Legacy cmd
        //AES132_KEY_CONFIG_AUTH_KEY | // Prior authentication not required 
        AES132_KEY_CONFIG_CHILD | // Allow update via KeyCreate/KeyLoad 
        //AES132_KEY_CONFIG_PARENT | // VolatileKey isn't used  
        //AES132_KEY_CONFIG_CHANGE_KEYS | // Cannot be overwrited via EncWrite
        0;
    config_stretch[1] =
        //AES132_KEY_CONFIG_COUNTER_LIMIT | // No usage limit
        //AES132_KEY_CONFIG_CHILD_MAC | // Irrelevant; KeyCreate can't target
        //AES132_KEY_CONFIG_AUTH_OUT | // I2C auth signaling disabled
        //AES132_KEY_CONFIG_AUTH_OUT_HOLD | // Irrelevant; Auth signaling disabled
        //AES132_KEY_CONFIG_IMPORT_OK | // Prohibit KeyImport
        //AES132_KEY_CONFIG_EXPORT_AUTH | // Irreleant; KeyCreate can't target
        //AES132_KEY_CONFIG_TRANSFER_OK | // Prohibit KeyTransfer
        //AES132_KEY_CONFIG_AUTH_COMPUTE | // Key cannot be used for auth_compute
        0;
    config_stretch[2] = 
        AES132_KEY_CONFIG_COUNTER_NUM(1) | // Not used; 1 for consistency
        AES132_KEY_CONFIG_LINK_POINTER(0) ; // Not used
    config_stretch[3] = 
        //AES_132_KEY_CONFIG_DEC_READ | // DecRead and WriteCompute prohibited
        0;
    res = aes132m_write_memory(sizeof(config_stretch),
            AES132_KEY_CONFIG_ADDR(AES132_KEY_ID_STRETCH), &config_stretch);
    if( res ) {
        ESP_LOGE(TAG, "Failed writing Stretch KeyConfig "
                "Error Return Code 0x%02X",
                res);
        return res;
    }

    uint8_t config_pin[4] = { 0 };
    config_pin[0] = 
        //AES132_KEY_CONFIG_EXTERNAL_CRYPTO | // Prohibit Encrypt/Decrypt
        AES132_KEY_CONFIG_INBOUND_AUTH | // Key only used for authentication
        AES132_KEY_CONFIG_RANDOM_NONCE | // Prevent Spoofing of Nonces 
        //AES132_KEY_CONFIG_LEGACY_OK | // Never allow dangerous Legacy cmd
        //AES132_KEY_CONFIG_AUTH_KEY | // Prior authentication not required 
        AES132_KEY_CONFIG_CHILD | // Allow update via KeyCreate/KeyLoad 
        //AES132_KEY_CONFIG_PARENT | // VolatileKey isn't used  
        //AES132_KEY_CONFIG_CHANGE_KEYS | // Cannot be overwrited via EncWrite
        0;
    config_pin[1] =
        AES132_KEY_CONFIG_COUNTER_LIMIT | // Limit Key Usage
        //AES132_KEY_CONFIG_CHILD_MAC | // Irrelevant; KeyCreate can't target
        //AES132_KEY_CONFIG_AUTH_OUT | // I2C auth signaling disabled
        //AES132_KEY_CONFIG_AUTH_OUT_HOLD | // Irrelevant; Auth signaling disabled
        //AES132_KEY_CONFIG_IMPORT_OK | // Prohibit KeyImport
        //AES132_KEY_CONFIG_EXPORT_AUTH | // Irreleant; KeyCreate can't target
        //AES132_KEY_CONFIG_TRANSFER_OK | // Prohibit KeyTransfer
        //AES132_KEY_CONFIG_AUTH_COMPUTE | // Key cannot be used for auth_compute
        0;
    config_pin[2] = 
        AES132_KEY_CONFIG_COUNTER_NUM(0) | // To be updated in loopd
        AES132_KEY_CONFIG_LINK_POINTER(0) ; // Not used
    config_pin[3] = 
        //AES_132_KEY_CONFIG_DEC_READ | // DecRead and WriteCompute prohibited
        0;
    res = aes132m_write_memory(sizeof(config_pin),
            AES132_KEY_CONFIG_ADDR(AES132_KEY_ID_PIN), &config_pin);
    if( res ) {
        ESP_LOGE(TAG, "Failed writing PIN KeyConfig "
                "Error Return Code 0x%02X",
                res);
        return res;
    }
    for(uint8_t i=AES132_KEY_ID_PIN; i < AES132_NUM_ZONES; i++) {
        config_pin[1] &= AES132_KEY_CONFIG_LINK_POINTER(0xF); // Clear out old counter_num
        config_pin[2] = AES132_KEY_CONFIG_COUNTER_NUM(i);
        res = aes132m_write_memory(sizeof(config_pin),
                AES132_KEY_CONFIG_ADDR(AES132_KEY_ID_PIN), &config_pin);
        if( res ) {
            ESP_LOGE(TAG, "Failed writing PIN (Key%d) KeyConfig "
                    "Error Return Code 0x%02X",
                    i, res);
            return res;
        }
    }
    return res;
}


uint8_t aes132_reset_master_zoneconfig() {
    /* Writes the ZoneConfig into a state so that we can easily write to
     * the UserZone before locking */
    uint8_t res;
    const uint8_t config_master[4] = { 0 };
    res = aes132m_write_memory(sizeof(config_master),
            AES132_ZONE_CONFIG_ADDR(AES132_KEY_ID_MASTER), &config_master);
    return res;
}

uint8_t aes132_write_zoneconfig() {
    uint8_t res;
    uint8_t config_master[4] = { 0 };
    config_master[0] = 
        //AES132_ZONE_CONFIG_AUTH_READ | // Master Zone just holding an esp32 encrypted backup of the master key, no authentication required to read the ciphertext.
        //AES132_ZONE_CONFIG_AUTH_WRITE | // Irrelevant; section is read-only
        //AES132_ZONE_CONFIG_ENC_READ | // No security benefit in EncRead
        //AES132_ZONE_CONFIG_ENC_WRITE | // Irrelevant; section is read-only
        AES132_ZONE_CONFIG_WRITE_MODE_4 | // Zone is read-only
        //AES132_ZONE_CONFIG_WRITE_MODE_5 |
        //AES132_ZONE_CONFIG_USE_SERIAL | // Irrelevant; section is read-only
        //AES132_ZONE_CONFIG_USE_SMALL | // Irrelevant; section is read-only
        0;
    config_master[1] = 
        AES132_ZONE_CONFIG_AUTH_ID(0) | // Master to generate OutMAC
        AES132_ZONE_CONFIG_READ_ID(0); // Irrelevant; authentication not required
    config_master[2] = 
        AES132_ZONE_CONFIG_WRITE_ID(x) | // Irrelevant, EncWrite not used
        //AES132_ZONE_CONFIG_VOLATILE_TRANSFER_OK | // Prohibit KeyTransfer to VolatileKey
        0;
    config_master[3] = 
         //AES132_ZONE_CONFIG_READ_ONLY_R | // Ignored unless WriteMode is 0b10 or 0b11
         0;
        
    res = aes132m_write_memory(sizeof(config_master),
            AES132_ZONE_CONFIG_ADDR(AES132_KEY_ID_MASTER), &config_master);
    if( res ) {
        ESP_LOGE(TAG, "Failed writing Master ZoneConfig "
                "Error Return Code 0x%02X",
                res);
        return res;
    }

    uint8_t config_pin[4] = { 0 };
    config_pin[0] = 
        AES132_ZONE_CONFIG_AUTH_READ | // PIN required to access zone
        //AES132_ZONE_CONFIG_AUTH_WRITE | // Irrelevant; section is read-only
        //AES132_ZONE_CONFIG_ENC_READ | // No security benefit in EncRead
        //AES132_ZONE_CONFIG_ENC_WRITE | // Irrelevant; section is read-only
        //AES132_ZONE_CONFIG_WRITE_MODE_4 |
        //AES132_ZONE_CONFIG_WRITE_MODE_5 | // Zone is R/W
        //AES132_ZONE_CONFIG_USE_SERIAL | // Irrelevant; section is read-only
        //AES132_ZONE_CONFIG_USE_SMALL | // Irrelevant; section is read-only
        0;
    config_pin[1] = 
        AES132_ZONE_CONFIG_AUTH_ID(0) | // Master to generate OutMAC
        AES132_ZONE_CONFIG_READ_ID(0); // Overwritten during write loop
    config_pin[2] = 
        AES132_ZONE_CONFIG_WRITE_ID(x) | // Irrelevant, EncWrite not used
        //AES132_ZONE_CONFIG_VOLATILE_TRANSFER_OK | // Prohibit KeyTransfer to VolatileKey
        0;
    config_pin[3] = 
         //AES132_ZONE_CONFIG_READ_ONLY_R | // Ignored unless WriteMode is 0b10 or 0b11
         0;
 
    for(uint8_t i=AES132_KEY_ID_PIN; i < 16; i++) {
        config_pin[1] &= AES132_ZONE_CONFIG_AUTH_ID(0xF); // Clear out old read_id
        config_pin[1] |= AES132_ZONE_CONFIG_READ_ID(i);
        res = aes132m_write_memory(sizeof(config_pin),
            AES132_ZONE_CONFIG_ADDR(i), &config_pin);
        if( res ) {
            ESP_LOGE(TAG, "Failed writing PIN (Key%d) ZoneConfig "
                    "Error Return Code 0x%02X",
                    i, res);
            return res;
        }
    }
    return res;
}
