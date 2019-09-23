#include "jolt_helpers.h"
#include "syscore/encoding.h"
#include "unity.h"

static const char MODULE_NAME[] = "[syscore/encoding/rle]";

TEST_CASE( "encoding & decoding", MODULE_NAME )
{
    int len;
    uint8_t buf[512]                   = {0};
    const char test_str_1[]            = "WWWWWWWWWWWWBWWWWWWWWWWWWBBBWWWWWWWWWWWWWWWWWWWWWWWWBWWWWWWWWWWWWWW";
    const uint8_t expected_encoded_1[] = {12, 'W', 1, 'B', 12, 'W', 3, 'B', 24, 'W', 1, 'B', 14, 'W'};

    /* Basic Use-Case */
    len = jolt_encoding_rle_encode( buf, sizeof( buf ), (uint8_t*)test_str_1, sizeof( test_str_1 ) );
    TEST_ASSERT_EQUAL_INT( sizeof( expected_encoded_1 ), len );
    TEST_ASSERT_EQUAL_MEMORY( expected_encoded_1, buf, sizeof( expected_encoded_1 ) );
    memzero( buf, sizeof( buf ) );

    len = jolt_encoding_rle_decode( buf, sizeof( buf ), expected_encoded_1, sizeof( expected_encoded_1 ) );
    TEST_ASSERT_EQUAL_INT( strlen( test_str_1 ), len );
    buf[len] = '\0';
    TEST_ASSERT_EQUAL_STRING( test_str_1, buf );

    // TODO test with 255 and  >255
}
