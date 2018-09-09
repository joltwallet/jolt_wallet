#include <string.h>                    // needed for memcpy()
#include "aes132_comm_marshaling.h"    // definitions and declarations for the Command Marshaling module
#include "aes132_i2c.h" // For ReturnCode macros
#include "esp_log.h"
#include "sodium.h"

/* Defines configuration and justification for Jolt */

bool aes132_write_chipconfig() {
    const aes132_chipconfig_t config = {
        .legacy = false, // Legacy command is not used; dangerous
        .encrypt_decrypt = ,
        .dec_read = ,
        .auth_compute = ,
        .reserved_0 = ,
        .power_up = 
    };
    uint8_t res;
    res = aes132m_write_memory(sizeof(config), AES132_CHIPCONFIG_BASE_ADDR,
            &config);
    return true; // todo: error handling overhaul
}

#if 0
bool aes132_write_counterconfig() {
    const aes132_counterconfig_t config = {
        .increment_ok = ,
        .require_mac = ,
        .reserved_0 = ,
        .incr_id = ,
        .mac_id = 
    };
    uint8_t res;
    res = aes132m_write_memory(sizeof(config), AES132_CONFIG_BASE_ADDR,
            &config);
    return true; // todo: error handling overhaul
}
#endif

#if 0
bool aes132_write_keyconfig() {
    const aes132_keyconfig_t config_master = {
        .external_crypto = ,
        .inbound_auth = ,
        .random_nonce = ,
        .legacy_ok = ,
        .auth_key = ,
        .child = ,
        .parent = ,
        .change_keys = ,
        .counter_limit = ,
        .child_mac = ,
        .auth_out = ,
        .auth_out_hold = ,
        .import_ok = ,
        .child_auth = ,
        .transfer_ok = ,
        .auth_compute = ,
        .link_pointer = ,
        .counter_num = ,
        .dec_read = ,
        .reserved_0 = 
    };
    const aes132_keyconfig_t config_stretch;
    const aes132_keyconfig_t config_pin;
    uint8_t res;
    res = aes132m_write_memory(sizeof(config), AES132_CONFIG_BASE_ADDR,
            &config);
    return true; // todo: error handling overhaul

}
#endif

#if 0
bool aes132_write_zoneconfig() {
    const aes132_config_t config = {
        .auth_read = ,
        .auth_write = ,
        .enc_read = ,
        .enc_write = ,
        .write_mode = ,
        .use_serial = ,
        .use_small = ,
        .read_id = , 
        .auth_id = ,
        .volatile_transfer_ok = ,
        .reserved_0 = ,
        .write_id = ,
        .read_only = 
    };
    uint8_t res;
    res = aes132m_write_memory(sizeof(config), AES132_CONFIG_BASE_ADDR,
            &config);
    return true; // todo: error handling overhaul
}
#endif
