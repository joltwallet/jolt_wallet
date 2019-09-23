#include "jolt_helpers.h"
#include "syscore/encoding.h"
#include "unity.h"

static const char MODULE_NAME[] = "[syscore/encoding/zlib]";

TEST_CASE( "encoding & decoding", MODULE_NAME )
{
    TEST_IGNORE();
    int len;
    uint8_t compressed[512]   = {0};
    uint8_t decompressed[512] = {0};
    const char test_str_1[]   = "The quick brown fox jumped over the lazy dog.";

    /* Basic Use-Case */
    len = jolt_encoding_zlib_encode( compressed, sizeof( compressed ), (uint8_t*)test_str_1, sizeof( test_str_1 ) );
    TEST_ASSERT_GREATER_THAN( 0, len );
    TEST_ASSERT_LESS_THAN( sizeof( compressed ) + 1, len );
    // TEST_ASSERT_EQUAL_MEMORY( expected_encoded_1, compressed, sizeof( expected_encoded_1 ) );

    printf( "\nCompressed data is %d long (%.2f%%).\n", len, ( (float)( 100 * len ) ) / sizeof( test_str_1 ) );

    len = jolt_encoding_rle_decode( decompressed, sizeof( decompressed ), compressed, len );
    TEST_ASSERT_EQUAL_INT( sizeof( test_str_1 ), len );
    TEST_ASSERT_EQUAL_STRING( test_str_1, decompressed );
}
