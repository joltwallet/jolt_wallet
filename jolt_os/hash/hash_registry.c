#include "algorithms/blake2b.h"
#include "algorithms/sha256.h"
#include "hash_internal.h"

const jolt_hash_entry_t jolt_hash_registry[] = {
        [JOLT_HASH_BLAKE2B] =
                {
                        .name   = "blake2b",
                        .size   = {.min = 1, .max = 64},
                        .single = jolt_hash_blake2b,
                        .init   = jolt_hash_blake2b_init,
                        .update = jolt_hash_blake2b_update,
                        .final  = jolt_hash_blake2b_final,
                },
        [JOLT_HASH_SHA256] =
                {
                        .name   = "sha256",
                        .size   = {.min = 32, .max = 32},
                        .single = jolt_hash_sha256,
                        .init   = jolt_hash_sha256_init,
                        .update = jolt_hash_sha256_update,
                        .final  = jolt_hash_sha256_final,
                },
};
