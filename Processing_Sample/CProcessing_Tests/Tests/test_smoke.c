// Phase A: the smallest possible test(s) to prove the harness works end to
// end -- Unity builds and runs under MSBuild, and the test executable links
// against and can call into the real CProcessing DLL.
#include "unity.h"
#include "cprocessing.h"

void test_smoke_truth(void)
{
    TEST_ASSERT_TRUE(1 == 1);
}

void test_smoke_cprocessing_link(void)
{
    TEST_ASSERT_EQUAL_INT(5, CP_Math_ClampInt(5, 0, 10));
}

// CP_Font_DrawText expects UTF-8. The builds pass /utf-8 to MSVC so a string
// such as "©" in a source file becomes UTF-8; without it, MSVC uses the
// computer's code page and the © comes out as a single, invalid byte (#15).
void test_smoke_source_strings_are_utf8(void)
{
    const unsigned char copyright[] = "©";
    TEST_ASSERT_EQUAL_size_t(3, sizeof(copyright));
    TEST_ASSERT_EQUAL_HEX8(0xC2, copyright[0]);
    TEST_ASSERT_EQUAL_HEX8(0xA9, copyright[1]);
}
