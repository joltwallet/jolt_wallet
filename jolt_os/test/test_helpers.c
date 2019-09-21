#include "jolt_helpers.h"
#include "unity.h"

static const char MODULE_NAME[] = "[jolt_helpers]";

TEST_CASE( "jolt_strcmp_suffix", MODULE_NAME ) {
    TEST_ASSERT_TRUE(  jolt_strcmp_suffix( "joltwallet", "wallet" ) );
    TEST_ASSERT_FALSE( jolt_strcmp_suffix( "cryptocurrency", "bitcoin") );
    TEST_ASSERT_TRUE(  jolt_strcmp_suffix( "cryptocurrency", "" ) );
    TEST_ASSERT_FALSE( jolt_strcmp_suffix( NULL, NULL) );
    TEST_ASSERT_FALSE(  jolt_strcmp_suffix( "cryptocurrency", NULL) );
    TEST_ASSERT_FALSE(  jolt_strcmp_suffix( NULL, "" ) );
    TEST_ASSERT_FALSE(  jolt_strcmp_suffix( "", "bitcoin" ) );
    TEST_ASSERT_TRUE(   jolt_strcmp_suffix( "", "") );
}

TEST_CASE( "jolt_bytes_to_hstr", MODULE_NAME ) {
}

TEST_CASE( "jolt_copy_until_space", MODULE_NAME ) {
}

