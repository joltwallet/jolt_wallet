#ifndef JOLT_CRYPTO_H__
#define JOLT_CRYPTO_H__

#include "jolt_hash.h"

enum {
    JOLT_CRYPTO_STATUS_SUCCESS = 0, /**< Hash computation successful */
    JOLT_CRYPTO_STATUS_FAIL,        /**< Generic failure */
    JOLT_CRYPTO_STATUS_NOT_IMPL,    /**< Functionality not implemented yet */
    JOLT_CRYPTO_STATUS_PARAM,       /**< Invalid input parameters */
    JOLT_CRYPTO_STATUS_OOM,         /**< Ran out of memory */
    JOLT_CRYPTO_STATUS_INSUFF_BUF,  /**< Output buffer is too small */
    JOLT_CRYPTO_STATUS_NOT_INIT,    /**< Current hashing state is not initialized */
    JOLT_CRYPTO_STATUS_INVALID_SIG, /**< Incorrect signature */
    JOLT_CRYPTO_STATUS_INVALID_PUB, /**< Invalid public key */
};
typedef uint8_t jolt_crypto_status_t;

enum {
    JOLT_CRYPTO_UNDEFINED = 0,
    JOLT_CRYPTO_ED25519,
    JOLT_CRYPTO_MAX_IDX,
};
typedef uint8_t jolt_crypto_type_t;

typedef struct {
    void *state; /**< Store state for signing specific algorithm */

    uint8_t *private;
    uint8_t *public;
    uint8_t *signature;

    uint16_t private_len;   /**< Private key length in bytes. 0 if key is not populated */
    uint16_t public_len;    /**< Public key length in bytes. 0 if key is not populated*/
    uint16_t signature_len; /**< Signature length. */

    jolt_hash_t hash;       /**< Jolt Hashing Context */
    jolt_crypto_type_t type;

    uint8_t initialized : 1;
} jolt_crypto_t;

/**
 * @brief Populates the public-key field from the private key.
 * If private key isnt valid, but of correct length,
 * will deterministically modify it to become a valid private key.
 *
 * The following fields must be populated prior to calling:
 *    1. `ctx->type` Signature algorithm to use.
 *    2. `ctx->hash.type` Hashing algorithm to use in the signing process. Leaving
 *       this to JOLT_HASH_UNDEFINED will default to the standard hashing algorithm
 *       for the selected signature.
 *    3. `ctx->public` and `ctx->public_len` must be set.
 *
 * @param[in] ctx Jolt signing context.
 * @return Status return code.
 */
jolt_crypto_status_t jolt_crypto_derive( jolt_crypto_t *ctx );

/**
 * @brief Sign a message.
 *
 * @param[in] sig_type Signature algorithm to use.
 * @param[in] hash_type Hashing algorithm to use. JOLT_HASH_UNDEFINED for signing algorithm's default.
 * @param[out] sig Output signature buffer.
 * @param[in] sig_len Output signature buffer length.
 * @param[in] msg Input message buffer to sign.
 * @param[in] msg_len Length of input message buffer.
 * @param[in] private_key Private Key
 * @param[in] private_key_len Length of private key.
 * @return Status return code.
 */
jolt_crypto_status_t jolt_crypto_sign( jolt_crypto_type_t sig_type, jolt_hash_type_t hash_type, uint8_t *sig,
                                       uint16_t sig_len, const uint8_t *msg, size_t msg_len,
                                       const uint8_t *private_key, uint16_t private_key_len );

/**
 * @brief Verify a message
 *
 * @param[in] sig_type Signature algorithm to use.
 * @param[in] hash_type Hashing algorithm to use. JOLT_HASH_UNDEFINED for signing algorithm's default.
 * @param[in] sig Input signature buffer.
 * @param[in] sig_len Input signature buffer length.
 * @param[in] msg Input message buffer to sign.
 * @param[in] msg_len Length of input message buffer.
 * @param[in] public_key Public Key
 * @param[in] public_key_len Length of public key.

 * @return Status return code. Returns `JOLT_CRYPTO_STATUS_INVALID_SIG` if everything
 *         was performed correctly, but the signature doesn't match the message and
 *         public key.
 */
jolt_crypto_status_t jolt_crypto_verify( jolt_crypto_type_t sig_type, jolt_hash_type_t hash_type, const uint8_t *sig,
                                         uint16_t sig_len, const uint8_t *msg, size_t msg_len,
                                         const uint8_t *public_key, uint16_t public_key_len );

/**
 * @brief Initialize a multi-part signature signing session.
 *
 * The following fields must be populated prior to calling:
 *    1. `ctx->type` Signature algorithm to use.
 *    2. `ctx->hash.type` Hashing algorithm to use in the signing process. Leaving
 *       this to JOLT_HASH_UNDEFINED will default to the standard hashing algorithm
 *       for the selected signature.
 *
 * @param[in] ctx Jolt signing context
 * @return Status return code.
 */
jolt_crypto_status_t jolt_crypto_sign_init( jolt_crypto_t *ctx );

/**
 * @brief Initialize a multi-part signature verificiation session.
 *
 * The following fields must be populated prior to calling:
 *    1. `ctx->type` Signature algorithm to use.
 *    2. `ctx->hash.type` Hashing algorithm to use in the signing process. Leaving
 *       this to JOLT_HASH_UNDEFINED will default to the standard hashing algorithm
 *       for the selected signature.
 *    3. `ctx->signature` `ctx->signature_len`
 *
 * @param[in] ctx Jolt signing context
 * @return Status return code.
 */
jolt_crypto_status_t jolt_crypto_verify_init( jolt_crypto_t *ctx );

/**
 * @brief Add part of a message to the digest.
 *
 * @param[in] ctx Jolt signing context
 * @param[in] msg Message portion to sign.
 * @param[in] msg_len Length of message portion in bytes.
 * @return Status return code.
 */
jolt_crypto_status_t jolt_crypto_sign_update( jolt_crypto_t *ctx, const uint8_t *msg, size_t msg_len );

/**
 * @brief Add part of a message to the digest.
 *
 * @param[in] ctx Jolt signing context
 * @param[in] msg Message portion to verify.
 * @param[in] msg_len Length of message portion in bytes.
 * @return Status return code.
 */
jolt_crypto_status_t jolt_crypto_verify_update( jolt_crypto_t *ctx, const uint8_t *msg, size_t msg_len );

/**
 * @brief Finalize and verify the signature.
 *
 * The following fields must be populated prior to calling:
 *    1. `ctx->public_key` and `ctx->public_key_len`
 *    2. `ctx->signature` and `ctx->signature_len`
 *
 * Typical work flow is:
 *    1. Create and populate context. Allocate your own buffers (or use static)
 *       and provide the pointers to them in the context.
 *    2. Call `jolt_crypto_init()`
 *    3. Feed in all parts of message via `jolt_crypto_update()`
 *    4. Generate the signature via jolt_`crypto_verify_final()`.
 *
 * @param[in] ctx Jolt signing context
 * @return Status return code. Returns `JOLT_CRYPTO_STATUS_INVALID_SIG` if everything
 *         was performed correctly, but the signature doesn't match the message and
 *         public key.
 */
jolt_crypto_status_t jolt_crypto_verify_final( jolt_crypto_t *ctx );

/**
 * @brief Finalize and generate the signature
 *
 * @param[in] ctx Jolt signing context
 * @return Status return code.
 */
jolt_crypto_status_t jolt_crypto_sign_final( jolt_crypto_t *ctx );

/**
 * @brief Converts an alogrithm name-string to a valid jolt identifier.
 * @param[in] name Signature name. Gets matched to the `name` field in the Jolt Signature Registry.
 * @return Signing algorithm type. Returns `JOLT_CRYPTO_UNDEFINED` (0) if not found.
 */
jolt_crypto_type_t jolt_crypto_from_str( const char *name );

#endif
