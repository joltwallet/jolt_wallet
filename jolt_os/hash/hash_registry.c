#include "algorithms/blake2b.h"
#include "algorithms/sha256.h"
#include "algorithms/sha512.h"
#include "algorithms/ripemd160.h"
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
        [JOLT_HASH_SHA512] =
                {
                        .name   = "sha512",
                        .size   = {.min = 64, .max = 64},
                        .single = jolt_hash_sha512,
                        .init   = jolt_hash_sha512_init,
                        .update = jolt_hash_sha512_update,
                        .final  = jolt_hash_sha512_final,
                },
        [JOLT_HASH_RIPEMD160] =
                {
                        .name   = "ripemd160",
                        .size   = {.min = 20, .max = 20},
                        .single = jolt_hash_ripemd160,
                        .init   = jolt_hash_ripemd160_init,
                        .update = jolt_hash_ripemd160_update,
                        .final  = jolt_hash_ripemd160_final,
                },
};
