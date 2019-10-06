#ifndef JOLT_HASH_INTERNAL_H__
#define JOLT_HASH_INTERNAL_H__

#include "jolt_hash.h"

typedef jolt_hash_status_t (*jolt_hash_func_single_t)(
        uint8_t *hash, size_t hash_len,
        const uint8_t *msg, size_t msg_len,
        const uint8_t *key, uint8_t key_len);

typedef jolt_hash_status_t (*jolt_hash_func_init_t)(jolt_hash_t *ctx); 
typedef jolt_hash_status_t (*jolt_hash_func_update_t)(jolt_hash_t *ctx, const uint8_t *msg, size_t msg_len); 
typedef jolt_hash_status_t (*jolt_hash_func_final_t)(jolt_hash_t *ctx); 

typedef struct {
    jolt_hash_func_single_t single;

    jolt_hash_func_init_t init;
    jolt_hash_func_update_t update;
    jolt_hash_func_final_t final;
    
} jolt_hash_entry_t;

extern const jolt_hash_entry_t jolt_hash_registry[];

#endif
