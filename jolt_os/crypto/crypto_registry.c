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
};
