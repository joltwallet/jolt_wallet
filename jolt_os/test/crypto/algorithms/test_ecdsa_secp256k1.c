#include "jolt_crypto.h"
#include "jolt_hash.h"
#include "jolt_helpers.h"
#include "sodium.h"
#include "string.h"
#include "unity.h"

static const char MODULE_NAME[] = "[jolt_crypto/ecdsa_secp256k1]";

TEST_CASE( "ecdsa secp256k1 basic", MODULE_NAME )
{
    jolt_crypto_status_t status;
    uint8_t private[32], public[65], signature[73];
    uint16_t public_len    = sizeof( public );
    uint16_t signature_len = sizeof( signature );
    uint16_t bad_public_len;
    const char msg[] = "jolt wallet";
    uint8_t hash[32];

    /* Test invalid params */
    bad_public_len = 64;
    status = jolt_crypto_derive( JOLT_CRYPTO_ECDSA_SECP256K1, public, &bad_public_len, private, sizeof( private ) );
    TEST_ASSERT_JOLT_CRYPTO_STATUS( JOLT_CRYPTO_STATUS_PARAM, status );

    status = jolt_crypto_derive( JOLT_CRYPTO_ECDSA_SECP256K1, public, &public_len, private, 31 );
    TEST_ASSERT_JOLT_CRYPTO_STATUS( JOLT_CRYPTO_STATUS_PARAM, status );

    {
        /* Populate Private Key */
        const char hex[] = "1294d9f787ad6274b3814badd3871966ed6c11cd684feff1515f4014827a2d9e";
        int res;
        res = sodium_hex2bin( private, sizeof( private ), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );
    }

    /* Test Public Key Derivation */
    status = jolt_crypto_derive( JOLT_CRYPTO_ECDSA_SECP256K1, public, &public_len, private, sizeof( private ) );
    TEST_ASSERT_JOLT_CRYPTO_STATUS( JOLT_CRYPTO_STATUS_SUCCESS, status );

    {
        uint8_t expected_public[65];
        const char hex[] = "044bf1a09fd56e331f48690b76916eb21bd136f4b1de3b27b5c3d2c07d570fc746"
                           "8e031954e4273d603e7f7b8de2035b4cdfb39700373a61bc28d5441c5d482b50";
        int res;
        res = sodium_hex2bin( expected_public, sizeof( expected_public ), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );

        TEST_ASSERT_EQUAL_HEX8_ARRAY( expected_public, public, sizeof( expected_public ) );
    }

    /* Test Computed Signature */
    // First, hash the message
    {
        jolt_hash_status_t status;
        status = jolt_hash( JOLT_HASH_SHA256, hash, sizeof( hash ), (uint8_t*)msg, strlen( msg ), NULL, 0 );
        TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_SUCCESS, status );
    }

    status = jolt_crypto_sign( JOLT_CRYPTO_ECDSA_SECP256K1, signature, &signature_len, hash, sizeof( hash ), public,
                               sizeof( public ), private, sizeof( private ) );
    TEST_ASSERT_JOLT_CRYPTO_STATUS( JOLT_CRYPTO_STATUS_SUCCESS, status );
    jolt_print_bytearray( signature, signature_len, false );  // debug

    {
        /* DER encoded */
        uint8_t expected_signature[71];
        const char hex[] = "3045022100cb3c0263071c8caebbd6699e5e0c12d38fadbd4561522b8a579e9fc29b6f1"
                           "00f022050eb309dc6f35b52d554d64cd0ec4dc32b32adf235ed0bec77296e6ca935c078";
        int res;
        res = sodium_hex2bin( expected_signature, sizeof( expected_signature ), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );

        // TEST_ASSERT_EQUAL_INT(sizeof(expected_signature), signature_len);
        // TEST_ASSERT_EQUAL_HEX8_ARRAY( expected_signature, signature, sizeof(expected_signature) );
    }

    /* Verify Signature */
    status = jolt_crypto_verify( JOLT_CRYPTO_ECDSA_SECP256K1, signature, signature_len, hash, sizeof( hash ), public,
                                 sizeof( public ) );
    TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_SUCCESS, status );
}
