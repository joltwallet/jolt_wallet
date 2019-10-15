#include "algorithms/ecdsa_secp256k1.h"
#include "algorithms/ed25519.h"
#include "crypto_internal.h"

const jolt_crypto_entry_t jolt_crypto_registry[] = {
        [JOLT_CRYPTO_ED25519] =
                {
                        .name   = "ed25519",
                        .derive = jolt_crypto_ed25519_derive,
                        .sign   = jolt_crypto_ed25519_sign,
                        .verify = jolt_crypto_ed25519_verify,
                },
        [JOLT_CRYPTO_ECDSA_SECP256K1] =
                {
                        .name   = "ecdsa_secp256k1",
                        .derive = jolt_crypto_ecdsa_secp256k1_derive,
                        .sign   = jolt_crypto_ecdsa_secp256k1_sign,
                        .verify = jolt_crypto_ecdsa_secp256k1_verify,
                },

};
