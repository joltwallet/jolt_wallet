#ifndef JOLT_CRYPTO_ALGORITHMS_ECDSA_H__
#define JOLT_CRYPTO_ALGORITHMS_ECDSA_H__

#include <mbedtls/ecdsa.h>
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
 * @brief Create a signature. In DER format. Namely:
 *    0 - Always 0x30
 *    1 - Number of bytes that follows (i.e. total length - 2 )
 *    2 - 0x02
 *    3 - Length of R
 *    4: - R
 *    4+len(R) - 0x02
 *    4+len(R)+1 - Length of S
 *    4+len(R)+2 - S
 *
 * The `sig` buffer should be at least 73 bytes long.
 * @param[out] sig
 * @param[in,out] sig_len Pass in signature buffer length; return actual length of signature.
 * @param[in] msg Hash to sign. Must be 32 bytes long.
 * @param[in] msg_len Length of hash. Must be 32.
 * @param[in] public_key Public key in uncompressed format. Must be 65 bytes long.
 * @param[in] public_key_len Public key length. Must be 65 bytes.
 * @param[in] private_key Private Key. Must be 32 bytes long.
 * @param[in] private_key_len Private Key length. Must be 32.
 */
jolt_crypto_status_t jolt_crypto_ecdsa_secp256k1_sign( uint8_t *sig, uint16_t *sig_len, const uint8_t *msg,
                                                       size_t msg_len, const uint8_t *public_key,
                                                       uint16_t public_key_len, const uint8_t *private_key,
                                                       uint16_t private_key_len );

/**
 * @brief Verify a signature in DER format
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

jolt_crypto_status_t jolt_crypto_ecdsa_secp256k1_keypair( mbedtls_ecp_keypair *keypair, const uint8_t *private_key );
#endif
