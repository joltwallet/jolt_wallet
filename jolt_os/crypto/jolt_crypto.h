#ifndef JOLT_CRYPTO_H__
#define JOLT_CRYPTO_H__

#include "stdint.h"
#include "stddef.h"

enum {
    JOLT_CRYPTO_STATUS_SUCCESS = 0, /**< Successful */
    JOLT_CRYPTO_STATUS_FAIL,        /**< Generic failure */
    JOLT_CRYPTO_STATUS_NOT_IMPL,    /**< Functionality not implemented yet */
    JOLT_CRYPTO_STATUS_PARAM,       /**< Invalid input parameters */
    JOLT_CRYPTO_STATUS_OOM,         /**< Ran out of memory */
    JOLT_CRYPTO_STATUS_INSUFF_BUF,  /**< Output buffer is too small */
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

/**
 * @brief Populates the public-key field from the private key.
 *
 * @param[in] type Signature algorithm to use.
 * @param[out] public_key Public Key
 * @param[in] public_key_len Length of public key.
 * @param[in] private_key Private Key
 * @param[in] private_key_len Length of private key.
 * @return Status return code.
 */
jolt_crypto_status_t jolt_crypto_derive( jolt_crypto_type_t type,
        uint8_t *public_key, uint16_t public_key_len,
        const uint8_t *private_key, uint16_t private_key_len );

/**
 * @brief Sign a message.
 *
 * @param[in] type Signature algorithm to use.
 * @param[out] sig Output signature buffer.
 * @param[in] sig_len Output signature buffer length.
 * @param[in] msg Input message buffer to sign.
 * @param[in] msg_len Length of input message buffer.
 * @param[in] private_key Private Key
 * @param[in] private_key_len Length of private key.
 * @return Status return code.
 */
jolt_crypto_status_t jolt_crypto_sign( jolt_crypto_type_t type, uint8_t *sig,
                                       uint16_t sig_len, const uint8_t *msg, size_t msg_len,
                                       const uint8_t *public_key, uint16_t public_key_len,
                                       const uint8_t *private_key, uint16_t private_key_len );

/**
 * @brief Verify a message
 *
 * @param[in] type Signature algorithm to use.
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
jolt_crypto_status_t jolt_crypto_verify( jolt_crypto_type_t type, const uint8_t *sig,
                                         uint16_t sig_len, const uint8_t *msg, size_t msg_len,
                                         const uint8_t *public_key, uint16_t public_key_len );

/**
 * @brief Converts an alogrithm name-string to a valid jolt identifier.
 * @param[in] name Signature name. Gets matched to the `name` field in the Jolt Signature Registry.
 * @return Signing algorithm type. Returns `JOLT_CRYPTO_UNDEFINED` (0) if not found.
 */
jolt_crypto_type_t jolt_crypto_from_str( const char *name );

#endif
