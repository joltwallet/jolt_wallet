#include "algorithms/blake2b.h"
#include "algorithms/sha256.h"
#include "hash_internal.h"

const jolt_hash_entry_t jolt_hash_registry[] = {
        [JOLT_HASH_BLAKE2B] =
                {
                        .single = jolt_hash_blake2b,
                        .init   = jolt_hash_blake2b_init,
                        .update = jolt_hash_blake2b_update,
                        .final  = jolt_hash_blake2b_final,
                },
        [JOLT_HASH_SHA256] =
                {
                        .single = jolt_hash_sha256,
                        .init   = jolt_hash_sha256_init,
                        .update = jolt_hash_sha256_update,
                        .final  = jolt_hash_sha256_final,
                },
};
