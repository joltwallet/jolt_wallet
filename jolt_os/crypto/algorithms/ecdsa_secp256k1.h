#ifndef JOLT_CRYPTO_ALGORITHMS_ECDSA_H__
#define JOLT_CRYPTO_ALGORITHMS_ECDSA_H__

#include "jolt_crypto.h"

/**
 * @brief Derives public address in uncompressed format.
 *
 * Requires a 65 byte long public_key buffer.
 *     0 - Always 0x04
 *     1~32 - X
 *     33~64 - Y
 */
jolt_crypto_status_t jolt_crypto_ecdsa_secp256k1_derive( uint8_t *public_key, uint16_t *public_key_len,
                                                         const uint8_t *private_key, uint16_t private_key_len );

/**
 * @brief Create a signature.
 */
jolt_crypto_status_t jolt_crypto_ecdsa_secp256k1_sign( uint8_t *sig, uint16_t *sig_len, const uint8_t *msg,
                                                       size_t msg_len, const uint8_t *public_key,
                                                       uint16_t public_key_len, const uint8_t *private_key,
                                                       uint16_t private_key_len );

/**
 * @brief Verify a signature
 * @param[in] sig Signature to verify in DER format
 * @param[in] sig_len Signature length in bytes
 * @param[in] msg Hash to verify
 * @param[in] msg_len Length of hash. Must be 32.
 * @param[in] public_key Either the compressed or uncompressed public key (33/65 bytes long)
 * @param[in] public_key_len Must either be 33 or 65
 */
jolt_crypto_status_t jolt_crypto_ecdsa_secp256k1_verify( const uint8_t *sig, uint16_t sig_len, const uint8_t *msg,
                                                         size_t msg_len, const uint8_t *public_key,
                                                         uint16_t public_key_len );

#endif
