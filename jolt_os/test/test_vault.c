#include "vault.h"
#include "bipmnemonic.h"
#include "unity.h"

static const char MODULE_NAME[] = "[vault]";

TEST_CASE( "vault_str_to_purpose_type", MODULE_NAME )
{
    uint8_t res;
    uint32_t purpose, coin_type;

    res = vault_str_to_purpose_type("44'/165'", &purpose, &coin_type);
    TEST_ASSERT_EQUAL_UINT8(0, res);
    TEST_ASSERT_EQUAL_UINT32(44 | BM_HARDENED, purpose);
    TEST_ASSERT_EQUAL_UINT32(165 | BM_HARDENED, coin_type);

    res = vault_str_to_purpose_type("44/165", &purpose, &coin_type);
    TEST_ASSERT_EQUAL_UINT8(0, res);
    TEST_ASSERT_EQUAL_UINT32(44, purpose);
    TEST_ASSERT_EQUAL_UINT32(165, coin_type);

    res = vault_str_to_purpose_type("0/0", &purpose, &coin_type);
    TEST_ASSERT_EQUAL_UINT8(0, res);
    TEST_ASSERT_EQUAL_UINT32(0, purpose);
    TEST_ASSERT_EQUAL_UINT32(0, coin_type);
}

