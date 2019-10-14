#ifndef JOLT_CRYPTO_ALGORITHMS_ECDSA_H__
#define JOLT_CRYPTO_ALGORITHMS_ECDSA_H__

#include "jolt_crypto.h"

jolt_crypto_status_t jolt_crypto_ecdsa_secp256k1_derive( uint8_t *public_key, uint16_t *public_key_len,
                                                         const uint8_t *private_key, uint16_t private_key_len );

jolt_crypto_status_t jolt_crypto_ecdsa_secp256k1_sign( uint8_t *sig, uint16_t *sig_len, const uint8_t *msg,
                                                       size_t msg_len, const uint8_t *public_key,
                                                       uint16_t public_key_len, const uint8_t *private_key,
                                                       uint16_t private_key_len );

jolt_crypto_status_t jolt_crypto_ecdsa_secp256k1_verify( const uint8_t *sig, uint16_t sig_len, const uint8_t *msg,
                                                         size_t msg_len, const uint8_t *public_key,
                                                         uint16_t public_key_len );

#endif
