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
    int len;
    char buf[32] = { 0 };
    char *soln;

    /* Standard Usage Tests */
    #define TEST_HSTR_SUFFICIENT(solution, val, pre) \
        len = jolt_bytes_to_hstr( buf, sizeof(buf), val, pre ); \
        soln = solution; \
        TEST_ASSERT_EQUAL_INT(strlen(soln), len); \
        TEST_ASSERT_EQUAL_STRING(soln, buf);

    TEST_HSTR_SUFFICIENT("0 B", 0, 0);
    TEST_HSTR_SUFFICIENT("123 B", 123, 0);
    TEST_HSTR_SUFFICIENT("123.0 B", 123, 1);
    TEST_HSTR_SUFFICIENT("123.00 B", 123, 2);
    TEST_HSTR_SUFFICIENT("1023 B", 1023, 0);
    TEST_HSTR_SUFFICIENT("1 KB", 1024, 0);
    TEST_HSTR_SUFFICIENT("1 KB", 1025, 0);
    TEST_HSTR_SUFFICIENT("1.001 KB", 1025, 3);
    TEST_HSTR_SUFFICIENT("1024 KB", 1048575, 0);
    TEST_HSTR_SUFFICIENT("1023.999 KB", 1048575, 3); /* Edge-case where the resources to fix aren't worth it */
    TEST_HSTR_SUFFICIENT("1 MB", 1048576, 0);
    TEST_HSTR_SUFFICIENT("1024 MB", 1073741823, 0); /* Edge-case where the resources to fix aren't worth it */
    TEST_HSTR_SUFFICIENT("1 GB", 1073741824, 0);

    #undef TEST_HSTR_SUFFICIENT 

    /* Insufficient Buffer Tests */
    #define TEST_HSTR_INSUFFICIENT(solution, val, pre, buf_size) \
        memzero(buf, sizeof(buf)); \
        len = jolt_bytes_to_hstr( buf, buf_size, val, pre ); \
        soln = solution; \
        TEST_ASSERT_EQUAL_INT(strlen(soln), len); \
        TEST_ASSERT_EQUAL_MEMORY(solution, buf, buf_size-1);\
        TEST_ASSERT_EQUAL_INT8('\0', buf[buf_size-1]);

    #define TEST_HSTR_SUFFICIENT(solution, val, pre, buf_size) \
        len = jolt_bytes_to_hstr( buf, buf_size, val, pre ); \
        soln = solution; \
        TEST_ASSERT_EQUAL_INT(strlen(soln), len); \
        TEST_ASSERT_EQUAL_STRING(soln, buf);

    TEST_HSTR_INSUFFICIENT("0 B", 0, 0, 2);
    TEST_HSTR_INSUFFICIENT("0 B", 0, 0, 3);
    TEST_HSTR_SUFFICIENT("0 B", 0, 0, 4);
    TEST_HSTR_INSUFFICIENT("10 B", 10, 0, 4);
    TEST_HSTR_SUFFICIENT("10 B", 10, 0, 5);
    TEST_HSTR_INSUFFICIENT("10 KB", 1025*10, 0, 5);
    TEST_HSTR_SUFFICIENT("10 KB", 1025*10, 0, 6);

    #undef TEST_HSTR_INSUFFICIENT
    #undef TEST_HSTR_SUFFICIENT

    /* Other edge-cases */
    TEST_ASSERT_EQUAL_INT(3, jolt_bytes_to_hstr(NULL, 0, 0, 0));
}

TEST_CASE( "jolt_copy_until_space", MODULE_NAME ) {
}

