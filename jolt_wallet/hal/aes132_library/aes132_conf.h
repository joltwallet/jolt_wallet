#ifndef AES132_CONF_H
#define AES132_CONF_H

/* Makes configuration of ataes132a easier */

/* Configuration Memory Map */
#define AES132_SERIALNUM_ADDR         0xF000
#define AES132_LOTHISTORY_ADDR        0xF008
#define AES132_LOCKKEYS_ADDR          0xF020
#define AES132_LOCKSMALL_ADDR         0xF021
#define AES132_LOCKCONFIG_ADDR        0xF022
#define AES132_I2CADDR_ADDR           0xF040
#define AES132_CHIPCONFIG_ADDR        0xF041
#define AES132_COUNTERCONFIG_ADDR     0xF060
#define AES132_KEYCONFIG_ADDR         0xF080
#define AES132_ZONECONFIG_ADDR        0xF0C0

#define AES132_NUM_ZONES 16

#define AES132_INCLUDE_SMALLZONE false // We don't use smallzone
#define AES132_INCLUDE_SERIAL true // Good practice to include device serial
#define AES132_INCLUDE_COUNTER true // todo: come back and make a choice here
#define AES132_INCLUDE_SMALLZONE_SERIAL_COUNTER ( \
          (AES132_INCLUDE_SMALLZONE << 7) \
        | (AES132_INCLUDE_SERIAL << 6) \
        | (AES132_INCLUDE_COUNTER << 5) \
        )

#include "stdbool.h"

bool aes132_write_chipconfig();
bool aes132_write_counterconfig();
bool aes132_write_keyconfig();
bool aes132_write_zoneconfig();

/* For bitfields, LSB first; MSB last */

typedef union {
    uint8_t all;
    struct {
        uint8_t legacy: 1,
              encrypt_decrypt: 1,
              dec_read: 1,
              auth_compute: 1,
              reserved_0: 2,
              power_up: 2;
    };
} aes132_chipconfig_t;

typedef union {
    uint16_t all;
    struct {
        uint8_t increment_ok: 1,
                require_mac: 1,
                reserved_0: 6;
        uint8_t incr_id: 4,
              mac_id: 4;
    };
} aes132_counterconfig_t;

typedef union {
    uint32_t all;
    struct {
        uint8_t external_crypto: 1,
                inbound_auth: 1,
                random_nonce: 1,
                legacy_ok: 1,
                auth_key: 1,
                child: 1,
                parent: 1,
                change_keys: 1;
        uint8_t counter_limit: 1,
                child_mac: 1,
                auth_out: 1,
                auth_out_hold: 1,
                import_ok: 1,
                child_auth: 1,
                transfer_ok: 1,
                auth_compute: 1;
        uint8_t link_pointer: 4,
                counter_num: 4;
        uint8_t dec_read: 1,
                reserved_0: 7;
    };
} aes132_keyconfig_t;

#define AES132_ZONECONFIG_READONLY_RW 0x55
#define AES132_ZONECONFIG_READONLY_R 0x00

typedef union {
    uint32_t all;
    struct {
        uint8_t auth_read: 1,
                auth_write: 1,
                enc_read: 1,
                enc_write: 1,
                write_mode: 2,
                use_serial: 1,
                use_small: 1;
        uint8_t read_id: 4, 
                auth_id: 4;
        uint8_t volatile_transfer_ok: 1,
                reserved_0: 3,
                write_id: 4;
        uint8_t read_only: 8;
    };
} aes132_zoneconfig_t;

#endif
