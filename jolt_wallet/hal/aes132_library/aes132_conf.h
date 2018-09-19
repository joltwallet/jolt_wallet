#ifndef AES132_CONF_H
#define AES132_CONF_H

/* Makes configuration of ataes132a easier */
#include "aes132_mac.h"

#define AES132_LOCK_SMALLZONE     0b00
#define AES132_LOCK_KEY           0b01
#define AES132_LOCK_CONFIG        0b10
#define AES132_LOCK_ZONECONFIG_RO 0b11

#define AES132_EEPROM_RNG_UPDATE (1 << 1)

#define AES132_MACFLAG_DEVICE 0x01 // For mac vlaues output by the ATAES132a
#define AES132_MACFLAG_HOST 0x03 // For MAC values sent to the device as inputs

#include "stdbool.h"

uint8_t aes132_write_chipconfig();
uint8_t aes132_write_counterconfig();
uint8_t aes132_write_keyconfig();
uint8_t aes132_write_zoneconfig();

uint8_t aes132_reset_master_zoneconfig();

#endif
