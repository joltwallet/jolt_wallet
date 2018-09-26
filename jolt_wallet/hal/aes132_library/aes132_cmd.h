#ifndef __AES132_CMD_H__
#define __AES132_CMD_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "aes132_mac.h"
#include "jolttypes.h"

uint8_t aes132_auth(const uint8_t *key, uint16_t key_id,
        struct aes132h_nonce_s *nonce);
uint8_t aes132_blockread(uint8_t *data, const uint16_t address, 
        const uint8_t count);
uint8_t aes132_check_configlock(bool *status);
uint8_t aes132_counter(const uint8_t *mac_key, uint32_t *count,
        uint8_t counter_id, struct aes132h_nonce_s *nonce);
uint8_t aes132_encrypt(const uint8_t *in, uint8_t len, uint8_t key_id, 
        uint8_t *out_data, uint8_t *out_mac, struct aes132h_nonce_s *nonce);
uint8_t aes132_key_create(uint8_t key_id, struct aes132h_nonce_s *nonce);
uint8_t aes132_key_load(const uint128_t parent_key,uint128_t child_key,
        const uint8_t key_id, struct aes132h_nonce_s *nonce);
uint8_t aes132_legacy(const uint8_t key_id, uint8_t *data);
uint8_t aes132_lock_device();
uint8_t aes132_mac_count(uint8_t *count);
uint8_t aes132_mac_incr(struct aes132h_nonce_s *nonce);
uint8_t aes132_nonce(struct aes132h_nonce_s *nonce, const uint8_t *in_seed);
uint8_t aes132_rand(uint8_t *out, const size_t n_bytes);

#endif
