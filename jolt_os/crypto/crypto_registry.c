#include "algorithms/ed25519.h"
#include "crypto_internal.h"

const jolt_crypto_entry_t jolt_crypto_registry[] = {
        [JOLT_CRYPTO_ED25519] =
                {
                        .name          = "ed25519",
                        .default_hash  = JOLT_HASH_SHA512,
                        .private_len   = 32,
                        .public_len    = 32,
                        .signature_len = 64,

                        .derive       = jolt_crypto_ed25519_derive,

                        .sign_init         = jolt_crypto_ed25519_sign_init,
                        .verify_init         = jolt_crypto_ed25519_verify_init,

                        .sign_update       = jolt_crypto_ed25519_sign_update,
                        .verify_update       = jolt_crypto_ed25519_verify_update,

                        .sign_final   = jolt_crypto_ed25519_sign_final,
                        .verify_final = jolt_crypto_ed25519_verify_final,
                },
};
