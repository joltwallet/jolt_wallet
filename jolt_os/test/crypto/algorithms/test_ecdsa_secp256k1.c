#include "jolt_crypto.h"
#include "sodium.h"
#include "string.h"
#include "unity.h"

static const char MODULE_NAME[] = "[jolt_crypto/ecdsa_secp256k1]";

TEST_CASE( "ecdsa secp256k1 basic", MODULE_NAME )
{
    /* https://tools.ietf.org/html/rfc6979#appendix-A.2 */

    jolt_crypto_status_t status;
    uint8_t private[32], public[64], signature[64];
    uint16_t public_len    = sizeof( public );
    uint16_t signature_len = sizeof( signature );
    uint16_t bad_public_len;
    const uint8_t msg[] = {0x72};

    /* Test invalid params */
    bad_public_len = 63;
    status = jolt_crypto_derive( JOLT_CRYPTO_ECDSA_SECP256K1, public, &bad_public_len, private, sizeof( private ) );
    TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_PARAM, status );

    status = jolt_crypto_derive( JOLT_CRYPTO_ECDSA_SECP256K1, public, &public_len, private, 31 );
    TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_PARAM, status );

    {
        /* Populate Private Key */
        const char hex[] = "AA5E28D6A97A2479A65527F7290311A3624D4CC0FA1578598EE3C2613BF99522";
        int res;
        res = sodium_hex2bin( private, sizeof( private ), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );
    }

    /* Test Public Key Derivation */
    status = jolt_crypto_derive( JOLT_CRYPTO_ECDSA_SECP256K1, public, &public_len, private, sizeof( private ) );
    TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_SUCCESS, status );

    {
        uint8_t expected_public[64];
        const char hex[] = "34F9460F0E4F08393D192B3C5133A6BA099AA0AD9FD54EBCCFACDFA239FF49C6"
                           "0B71EA9BD730FD8923F6D25A7A91E7DD7728A960686CB5A901BB419E0F2CA232";
        int res;
        res = sodium_hex2bin( expected_public, sizeof( expected_public ), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );

        TEST_ASSERT_EQUAL_HEX8_ARRAY( expected_public, public, 64 );
    }

    /* Test Computed Signature */
#if 0
    status = jolt_crypto_sign( JOLT_CRYPTO_ED25519, signature, sizeof( signature ), msg, sizeof( msg ), public,
                               sizeof( public ), private, sizeof( private ) );
    TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_SUCCESS, status );

    {
        uint8_t expected_signature[64];
        const char hex[] = "92a009a9f0d4cab8720e820b5f642540"
                           "a2b27b5416503f8fb3762223ebdb69da"
                           "085ac1e43e15996e458f3613d0f11d8c"
                           "387b2eaeb4302aeeb00d291612bb0c00";
        int res;
        res = sodium_hex2bin( expected_signature, sizeof( expected_signature ), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );

        TEST_ASSERT_EQUAL_HEX8_ARRAY( expected_signature, signature, sizeof( signature ) );
    }

    /* Verify Signature */
    status = jolt_crypto_verify( JOLT_CRYPTO_ED25519, signature, sizeof( signature ), msg, sizeof( msg ), public,
                                 sizeof( public ) );
    TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_SUCCESS, status );
#endif
}
