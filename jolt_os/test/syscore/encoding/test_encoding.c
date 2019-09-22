#include "unity.h"
#include "string.h"
#include "syscore/encoding.h"

static const char MODULE_NAME[] = "[syscore/encoding/encoding]";

TEST_CASE( "NONE encoding & decoding", MODULE_NAME )
{
    int len;
    uint8_t buf[512] = { 0 };

    const char test_str_1[] = "test";

    /* Typical Use-Case */
    len = jolt_encoding_none_encode(buf, sizeof(buf), (uint8_t *)test_str_1, strlen(test_str_1) + 1);
    TEST_ASSERT_EQUAL_STRING(test_str_1, buf);
    TEST_ASSERT_EQUAL_INT(strlen(test_str_1) + 1, len);

    len = jolt_encoding_none_decode(buf, sizeof(buf), (uint8_t *)test_str_1, strlen(test_str_1) + 1);
    TEST_ASSERT_EQUAL_STRING(test_str_1, buf);
    TEST_ASSERT_EQUAL_INT(strlen(test_str_1) + 1, len);

    /* Edge Cases */
    len = jolt_encoding_none_encode(NULL, 0, (uint8_t *)test_str_1, strlen(test_str_1) + 1);
    TEST_ASSERT_EQUAL_INT(strlen(test_str_1) + 1, len);

    len = jolt_encoding_none_encode(NULL, 0, NULL, 0);
    TEST_ASSERT_EQUAL_INT(-1, len);

}
