#include "jolt_crypto.h"
#include "sodium.h"
#include "string.h"
#include "unity.h"

static const char MODULE_NAME[] = "[jolt_crypto]";

TEST_CASE( "jolt_crypto basic", MODULE_NAME )
{
    /* Using ED25519 for testing jolt_crypto api */
    /* https://tools.ietf.org/html/rfc8032#section-7.1 */
    jolt_crypto_status_t status;
    uint8_t private[32], public[32], signature[64];
    uint16_t public_len = sizeof( public );
    uint16_t bad_public_len;
    uint16_t signature_len = sizeof( signature );
    const uint8_t msg[]    = {0x72};

    /* Test invalid params */
    status = jolt_crypto_derive( JOLT_CRYPTO_UNDEFINED, public, &public_len, private, sizeof( private ) );
    TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_PARAM, status );

    status = jolt_crypto_derive( JOLT_CRYPTO_ED25519, NULL, &public_len, private, sizeof( private ) );
    TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_PARAM, status );

    bad_public_len = 0;
    status         = jolt_crypto_derive( JOLT_CRYPTO_ED25519, public, &bad_public_len, private, sizeof( private ) );
    TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_PARAM, status );

    bad_public_len = 31;
    status         = jolt_crypto_derive( JOLT_CRYPTO_ED25519, public, &bad_public_len, private, sizeof( private ) );
    TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_PARAM, status );

    status = jolt_crypto_derive( JOLT_CRYPTO_ED25519, public, NULL, private, sizeof( private ) );
    TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_PARAM, status );

    status = jolt_crypto_derive( JOLT_CRYPTO_ED25519, public, &public_len, NULL, sizeof( private ) );
    TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_PARAM, status );

    status = jolt_crypto_derive( JOLT_CRYPTO_ED25519, public, &public_len, private, 0 );
    TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_PARAM, status );

    status = jolt_crypto_derive( JOLT_CRYPTO_ED25519, public, &public_len, private, 31 );
    TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_PARAM, status );

    {
        /* Populate Private Key */
        const char hex[] = "4ccd089b28ff96da9db6c346ec114e0f5b8a319f35aba624da8cf6ed4fb8a6fb";
        int res;
        res = sodium_hex2bin( private, sizeof( private ), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );
    }

    /* Test Public Key Derivation */
    status = jolt_crypto_derive( JOLT_CRYPTO_ED25519, public, &public_len, private, sizeof( private ) );
    TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_SUCCESS, status );

    {
        uint8_t expected_public[32];
        const char hex[] = "3d4017c3e843895a92b70aa74d1b7ebc9c982ccf2ec4968cc0cd55f12af4660c";
        int res;
        res = sodium_hex2bin( expected_public, sizeof( expected_public ), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );

        TEST_ASSERT_EQUAL_HEX8_ARRAY( expected_public, public, 32 );
    }

    /* Test Computed Signature */
    status = jolt_crypto_sign( JOLT_CRYPTO_ED25519, signature, &signature_len, msg, sizeof( msg ), public,
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
}

TEST_CASE( "jolt_crypto_from_str", MODULE_NAME )
{
    jolt_crypto_type_t actual;

    actual = jolt_crypto_from_str( NULL );
    TEST_ASSERT_EQUAL_UINT8( JOLT_CRYPTO_UNDEFINED, actual );

    actual = jolt_crypto_from_str( "nonexistant" );
    TEST_ASSERT_EQUAL_UINT8( JOLT_CRYPTO_UNDEFINED, actual );

    actual = jolt_crypto_from_str( "ed25519" );
    TEST_ASSERT_EQUAL_UINT8( JOLT_CRYPTO_ED25519, actual );
}
