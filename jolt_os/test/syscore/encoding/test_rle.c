#include "unity.h"
#include "syscore/encoding.h"

static const char MODULE_NAME[] = "[syscore/encoding/rle]";

TEST_CASE( "encoding & decoding", MODULE_NAME )
{
    char buf[512] = { 0 };
    const char test_str_1[] = "WWWWWWWWWWWWBWWWWWWWWWWWWBBBWWWWWWWWWWWWWWWWWWWWWWWWBWWWWWWWWWWWWWW";
}
