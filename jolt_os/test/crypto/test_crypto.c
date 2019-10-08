#include "jolt_crypto.h"
#include "string.h"
#include "unity.h"

static const char MODULE_NAME[] = "[jolt_crypto]";

TEST_CASE( "jolt_crypto single", MODULE_NAME ) { TEST_IGNORE(); }

TEST_CASE( "jolt_crypto multi", MODULE_NAME ) { TEST_IGNORE(); }

TEST_CASE( "jolt_crypto_from_str", MODULE_NAME )
{
    jolt_hash_type_t actual;

    actual = jolt_crypto_from_str( NULL );
    TEST_ASSERT_EQUAL_UINT8( JOLT_CRYPTO_UNDEFINED, actual );

    actual = jolt_crypto_from_str( "nonexistant" );
    TEST_ASSERT_EQUAL_UINT8( JOLT_CRYPTO_UNDEFINED, actual );

    actual = jolt_crypto_from_str( "ed25519" );
    TEST_ASSERT_EQUAL_UINT8( JOLT_CRYPTO_ED25519, actual );
}
