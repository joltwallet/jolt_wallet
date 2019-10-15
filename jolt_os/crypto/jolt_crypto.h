#ifndef JOLT_CRYPTO_H__
#define JOLT_CRYPTO_H__

#include "stddef.h"
#include "stdint.h"

enum {
    JOLT_CRYPTO_STATUS_SUCCESS = 0,  /**< Successful */
    JOLT_CRYPTO_STATUS_FAIL,         /**< Generic failure */
    JOLT_CRYPTO_STATUS_NOT_IMPL,     /**< Functionality not implemented yet */
    JOLT_CRYPTO_STATUS_PARAM,        /**< Invalid input parameters */
    JOLT_CRYPTO_STATUS_OOM,          /**< Ran out of memory */
    JOLT_CRYPTO_STATUS_INSUFF_BUF,   /**< Output buffer is too small */
    JOLT_CRYPTO_STATUS_INVALID_SIG,  /**< Incorrect signature */
    JOLT_CRYPTO_STATUS_INVALID_PUB,  /**< Invalid public key */
    JOLT_CRYPTO_STATUS_INVALID_PRIV, /**< Invalid private key */
};
typedef uint8_t jolt_crypto_status_t;

enum {
    JOLT_CRYPTO_UNDEFINED = 0,
    JOLT_CRYPTO_ED25519,         /**< Raw 64-byte signature from 32 byte private and public keys */
    JOLT_CRYPTO_ECDSA_SECP256K1, /**< DER encoded signature up to 73 bytes long. 32 byte private, 64 byte public */
    JOLT_CRYPTO_MAX_IDX,
};
typedef uint8_t jolt_crypto_type_t;

/**
 * @brief Populates the public-key field from the private key.
 *
 * @param[in] type Signature algorithm to use.
 * @param[out] public_key Public Key
 * @param[in,out] public_key_len Length of public key. Passed in should be the buffer size. Returned is the actual
 * length.
 * @param[in] private_key Private Key
 * @param[in] private_key_len Length of private key.
 * @return Status return code.
 */
jolt_crypto_status_t jolt_crypto_derive( jolt_crypto_type_t type, uint8_t *public_key, uint16_t *public_key_len,
                                         const uint8_t *private_key, uint16_t private_key_len );

/**
 * @brief Sign a message.
 *
 * @param[in] type Signature algorithm to use.
 * @param[out] sig Output signature buffer.
 * @param[in,out] sig_len Output signature buffer length. Returned is the actual signature length.
 * @param[in] msg Input message buffer to sign.
 * @param[in] msg_len Length of input message buffer.
 * @param[in] private_key Private Key
 * @param[in] private_key_len Length of private key.
 * @return Status return code.
 */
jolt_crypto_status_t jolt_crypto_sign( jolt_crypto_type_t type, uint8_t *sig, uint16_t *sig_len, const uint8_t *msg,
                                       size_t msg_len, const uint8_t *public_key, uint16_t public_key_len,
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
jolt_crypto_status_t jolt_crypto_verify( jolt_crypto_type_t type, const uint8_t *sig, uint16_t sig_len,
                                         const uint8_t *msg, size_t msg_len, const uint8_t *public_key,
                                         uint16_t public_key_len );

/**
 * @brief Converts an alogrithm name-string to a valid jolt identifier.
 * @param[in] name Signature name. Gets matched to the `name` field in the Jolt Signature Registry.
 * @return Signing algorithm type. Returns `JOLT_CRYPTO_UNDEFINED` (0) if not found.
 */
jolt_crypto_type_t jolt_crypto_from_str( const char *name );

/**
 * @brief Convert enumerated status to string.
 * @param[in] status
 */
const char *jolt_crypto_status_to_str( jolt_crypto_status_t status );

/**
 * @brief UNITY Test
 */
#define TEST_ASSERT_JOLT_CRYPTO_STATUS( expected, actual )                                                          \
    {                                                                                                               \
        if( expected != actual ) {                                                                                  \
            printf( "Expected: %s (0x%02X) Actual: %s (0x%02X)\n", jolt_crypto_status_to_str( expected ), expected, \
                    jolt_crypto_status_to_str( actual ), actual );                                                  \
            TEST_FAIL();                                                                                            \
        }                                                                                                           \
    }

#endif
