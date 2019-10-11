#include "jolt_crypto.h"
#include "string.h"
#include "unity.h"
#include "sodium.h"

static const char MODULE_NAME[] = "[jolt_crypto]";

TEST_CASE( "jolt_crypto single", MODULE_NAME ) 
{ 
    /* https://tools.ietf.org/html/rfc8032#section-7.1 */
    jolt_crypto_status_t status;
    uint8_t private[32], public[32], signature[64];
    const uint8_t msg[] = { 0x72 };

    {
        /* Populate Private Key */
        const char hex[] = "9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60";
        int res;
        res = sodium_hex2bin( private, sizeof(private), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );
    }

    /* Test Public Key Derivation */
    status = jolt_crypto_derive(JOLT_CRYPTO_ED25519, public, sizeof(public), private, sizeof(private));
    TEST_ASSERT_EQUAL_HEX8(JOLT_CRYPTO_STATUS_SUCCESS, status);

    {
        uint8_t expected_public[32];
        const char hex[] = "d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a";
        int res;
        res = sodium_hex2bin( expected_public, sizeof(expected_public), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );

        TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_public, public, 32);
    }

    /* Test Computed Signature */
    status = jolt_crypto_sign(JOLT_CRYPTO_ED25519, 
            signature, sizeof(signature), 
            msg, sizeof(msg),
            public, sizeof(public),
            private, sizeof(private)
            );
    TEST_ASSERT_EQUAL_HEX8(JOLT_CRYPTO_STATUS_SUCCESS, status);

    {
        uint8_t expected_signature[64];
        const char hex[] = "e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e06522490155"
                  "5fb8821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b";
        int res;
        res = sodium_hex2bin( expected_signature, sizeof(expected_signature), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );

        TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_signature, signature, sizeof(signature));
    }

    /* Verify Signature */
    //TODO

}

TEST_CASE( "jolt_crypto multi", MODULE_NAME ) { TEST_IGNORE(); }

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
