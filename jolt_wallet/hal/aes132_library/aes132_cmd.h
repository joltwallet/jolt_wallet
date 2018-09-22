#ifndef __AES132_CMD_H__
#define __AES132_CMD_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

uint8_t aes132_check_configlock(bool *status);
uint8_t aes132_check_manufacturing_id(bool *status);
uint8_t lock_device();

uint8_t aes132_mac_count(uint8_t *count);
uint8_t aes132_rand(uint8_t *out, const size_t n_bytes);
uint8_t aes132_blockread(uint8_t *data, const uint16_t address, const uint8_t count);
uint8_t aes132_counter(uint32_t *count, uint8_t counter_id);

#endif
