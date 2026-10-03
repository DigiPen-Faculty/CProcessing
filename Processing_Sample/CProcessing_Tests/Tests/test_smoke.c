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
