#include <string.h>                    // needed for memcpy()
#include "aes132_comm_marshaling.h"    // definitions and declarations for the Command Marshaling module
#include "aes132_i2c.h" // For ReturnCode macros
#include "esp_log.h"
#include "sodium.h"


/* Defines configuration and justification for Jolt */
static const char TAG[] = "aes132_con";


uint8_t aes132_write_chipconfig() {
    const aes132_chipconfig_t config = {
        .legacy = false, // Legacy cmd is not used; dangerous
        .encrypt_decrypt = true, // Use Encrypt cmd for key stretching
        .dec_read = false, // DecRead and WriteCompute not used
        .auth_compute = false, //AuthCompute not used
        .power_up = 0b11 // active on startup; todo: optimize power consumption 
    };
    uint8_t res;
    res = aes132m_write_memory(sizeof(aes132_chipconfig_t),
            AES132_CHIPCONFIG_ADDR,
            (uint8_t *)&config);
    return res;
}

uint8_t aes132_write_counterconfig() {
    /* Only imposes restrictions on the usage of the Counter Command. 
     * Does not influence which keys use which counters. */
    uint8_t res;
    const aes132_counterconfig_t config = {
        .increment_ok = false, // No need to increment
        .require_mac = false, // irrelevant because of increment_ok = false
        .incr_id = 0b0000, // irrelevant because of increment_ok = false
        .mac_id = 0b0000 // Master Key ID
    };
    /* Write the same CounterConfig to all 16 registers */
    for(uint8_t i=0; i<AES132_NUM_ZONES; i++) {
        res = aes132m_write_memory(sizeof(aes132_counterconfig_t),
                AES132_COUNTERCONFIG_ADDR + i*sizeof(aes132_counterconfig_t),
                (uint8_t *)&config);
        if( res ) {
            ESP_LOGW(TAG, "Wrote up to %d CounterConfig. "
                    "Error 0x%02x on CounterConfig %d", i, res, i);
            return res;
        }
    }
    return res;
}

uint8_t aes132_write_keyconfig() {
    uint8_t res;
    /* Master is primarily used for inbound/outbound MACs */
    const aes132_keyconfig_t config_master = {
        .external_crypto = false, // Prohibit Encrypt/Decrypt
        .inbound_auth = false,    // Key not limited to only Auth command
        .random_nonce = true,     // Prevent spoofing of Nonces
        .legacy_ok = false,       // Never allow dangerous Legacy command
        .auth_key = false,        // Prior authentication not required
        .child = false,           // Prohibit update via KeyCreate/KeyLoad
        .parent = false,          // VolatileKey isn't used
        .change_keys = false,     // Cannot be overwrited via EncWrite
        .counter_limit = false,   // No usage limit
        .child_mac = false,       // Irrelevant; KeyCreate can't target
        .auth_out = false,        // I2C auth signaling disabled
        .auth_out_hold = false,   // Irrelevant; Auth signaling disabled
        .import_ok = false,       // Prohibit KeyImport
        .child_auth = false,      // Irreleant; KeyCreate can't target
        .transfer_ok = false,     // Prohibit KeyTransfer
        .auth_compute = false,    // Key cannot be used for auth_compute
        .link_pointer = 0x0,      // Not used
        .counter_num = 0x0,       // Not used
        .dec_read = false,        // DecRead and WriteCompute prohibited
    };

    res = aes132m_write_memory(sizeof(aes132_keyconfig_t),
            AES132_KEYCONFIG_ADDR + 0 * sizeof(aes132_keyconfig_t),
            (uint8_t *)&config_master);

    if( res ) {
        ESP_LOGE(TAG, "Failed writing Master (Key0) KeyConfig "
                "Error Return Code 0x%02X",
                res);
        return res;
    }
    

    /* Stretch is only used for Key streshing via Encrypt command */
    const aes132_keyconfig_t config_stretch = {
        .external_crypto = true,  // Allow Encrypt/Decrypt
        .inbound_auth = false,    // Key not limited to only Auth command
        .random_nonce = true,     // Prevent spoofing of Nonces
        .legacy_ok = false,       // Never allow dangerous Legacy command
        .auth_key = false,        // Prior authentication not required
        .child = true,            // Allow update via KeyCreate/KeyLoad
        .parent = false,          // VolatileKey isn't used
        .change_keys = false,     // Cannot be overwrited via EncWrite
        .counter_limit = false,   // No usage limit
        .child_mac = false,       // No InMAC for KeyCreate to modify this key
        .auth_out = false,        // I2C auth signaling disabled
        .auth_out_hold = false,   // Irrelevant; Auth signaling disabled
        .import_ok = false,       // Prohibit KeyImport; we use KeyCreate
        .child_auth = false,      // No prior auth for KeyCreate to modify this key
        .transfer_ok = false,     // Prohibit KeyTransfer
        .auth_compute = false,    // Key cannot be used for auth_compute
        .link_pointer = 0x0,      // Not used
        .counter_num = 0x1,       // Not used; stated as 0x1 for consistency
        .dec_read = false,        // DecRead and WriteCompute prohibited
    };
    res = aes132m_write_memory(sizeof(aes132_keyconfig_t),
            AES132_KEYCONFIG_ADDR + 1 * sizeof(aes132_keyconfig_t),
            (uint8_t *)&config_stretch);
    if( res ) {
        ESP_LOGE(TAG, "Failed writing Stretch (Key1) KeyConfig "
                "Error Return Code 0x%02X",
                res);
        return res;
    }

    aes132_keyconfig_t config_pin = {
        .external_crypto = false, // Prohibit Encrypt/Decrypt
        .inbound_auth = true,     // Key only used for authentication
        .random_nonce = true,     // Prevent spoofing of Nonces
        .legacy_ok = false,       // Never allow dangerous Legacy command
        .auth_key = false,        // Prior authentication not required
        .child = true,            // Allow update via KeyCreate/KeyLoad
        .parent = false,          // VolatileKey isn't used
        .change_keys = false,     // Cannot be overwrited via EncWrite
        .counter_limit = true,    // Limit Key Usage
        .child_mac = false,       // No InMAC for KeyCreate to modify this key
        .auth_out = false,        // I2C auth signaling disabled
        .auth_out_hold = false,   // Irrelevant; Auth signaling disabled
        .import_ok = false,       // Prohibit KeyImport; We use KeyLoad
        .child_auth = false,      // Irreleant; KeyCreate can't target
        .transfer_ok = false,     // Prohibit KeyTransfer
        .auth_compute = false,    // Key cannot be used for auth_compute
        .link_pointer = 0x0,      // Use Master for MAC
        .counter_num = 0x0,       // To be updated in loop
        .dec_read = false,        // DecRead and WriteCompute prohibited
    };
    for(uint8_t i=2; i < AES132_NUM_ZONES; i++) {
        config_pin.counter_num = i;
        res = aes132m_write_memory(sizeof(aes132_keyconfig_t),
                AES132_KEYCONFIG_ADDR + i * sizeof(aes132_keyconfig_t),
                (uint8_t *)&config_pin);
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
    const aes132_zoneconfig_t config_master = {
        .auth_read = false, // Master Zone just holding an esp32 encrypted backup of the master key, no authentication required to read the ciphertext.
        .auth_write = false, // Irrelevant; section is read-only
        .enc_read = false,   // No security benefit in EncRead
        .enc_write = false,  // Irrelevant; section is read-only
        .write_mode = 0b00,  // R/W
        .use_serial = AES132_INCLUDE_SERIAL,  // Irrelevant; section is read-only
        .use_small = AES132_INCLUDE_SMALLZONE,  // Irrelevant; section is read-only
        .read_id = 0x0,      // Master to generate OutMAC 
        .auth_id = 0x0,      // Irrelevant; authentication not required
        .volatile_transfer_ok = false, // Prohibit KeyTransfer to VolatileKey
        .write_id = 0x0,     // Irrelevant, EncWrite not used
        .read_only = 0x00  // Ignored unless WriteMode is 0b10 or 0b11
    };
    res = aes132m_write_memory(sizeof(aes132_zoneconfig_t),
            AES132_ZONECONFIG_ADDR + 0 * sizeof(aes132_zoneconfig_t),
            (uint8_t *)&config_master);
    return res;
}

uint8_t aes132_write_zoneconfig() {
    uint8_t res;
    const aes132_zoneconfig_t config_master = {
        .auth_read = false, // Master Zone just holding an esp32 encrypted backup of the master key, no authentication required to read the ciphertext.
        .auth_write = false, // Irrelevant; section is read-only
        .enc_read = false,   // No security benefit in EncRead
        .enc_write = false,  // Irrelevant; section is read-only
        .write_mode = 0b01,  // Zone is read-only
        .use_serial = AES132_INCLUDE_SERIAL,  // Irrelevant; section is read-only
        .use_small = AES132_INCLUDE_SMALLZONE,  // Irrelevant; section is read-only
        .read_id = 0x0,      // Master to generate OutMAC 
        .auth_id = 0x0,      // Irrelevant; authentication not required
        .volatile_transfer_ok = false, // Prohibit KeyTransfer to VolatileKey
        .write_id = 0x0,     // Irrelevant, EncWrite not used
        .read_only = 0x00  // Ignored unless WriteMode is 0b10 or 0b11
    };
    res = aes132m_write_memory(sizeof(aes132_zoneconfig_t),
            AES132_ZONECONFIG_ADDR + 0 * sizeof(aes132_zoneconfig_t),
            (uint8_t *)&config_master);
    if( res ) {
        ESP_LOGE(TAG, "Failed writing Master (Key1) ZoneConfig "
                "Error Return Code 0x%02X",
                res);
        return res;
    }

    aes132_zoneconfig_t config_pin = {
        .auth_read = true, // PIN required to access zone
        .auth_write = false, // Irrelevant; authentication for write doesn't
                             // provide more security
        .enc_read = false,   // No security benefit in EncRead
        .enc_write = false,  // No security benefit in EncWrite
        .write_mode = 0b10,  // Zone is read-only
        .use_serial = AES132_INCLUDE_SERIAL,  // Irrelevant; we don't use EncWrite
        .use_small = AES132_INCLUDE_SMALLZONE,  // Irrelevant; we don't use EncWrite
        .read_id = 0x0,      // Master to generate OutMAC 
        .auth_id = 0x0,      // Overwritten during write loop
        .volatile_transfer_ok = false, // Prohibit KeyTransfer to VolatileKey
        .write_id = 0x0,     // Irrelevant, EncWrite not used
        .read_only = AES132_ZONECONFIG_READONLY_RW  // Ignored unless WriteMode is 0b10 or 0b11
    };
    for(uint8_t i=2; i < AES132_NUM_ZONES; i++) {
        config_pin.auth_id = i;
        res = aes132m_write_memory(sizeof(aes132_zoneconfig_t),
                AES132_ZONECONFIG_ADDR + i * sizeof(aes132_zoneconfig_t),
                (uint8_t *)&config_pin);
        if( res ) {
            ESP_LOGE(TAG, "Failed writing PIN (Key%d) ZoneConfig "
                    "Error Return Code 0x%02X",
                    i, res);
            return res;
        }

    }
    return res;
}
