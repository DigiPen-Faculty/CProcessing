// Tier 1 coverage for Source/Internal_Platform.h, the small portability
// layer that replaced MSVC-only APIs (strcpy_s, MAX_PATH, ...) so the
// library builds on Linux and macOS. CP_StringCopy fills the image, sound
// and font file path caches, so truncation and termination matter.
#include <string.h>
#include "unity.h"
#include "Internal_Platform.h"

void test_platform_stringcopy_copies_short_string(void)
{
    char dest[16];
    memset(dest, 'x', sizeof(dest));
    CP_StringCopy(dest, sizeof(dest), "Assets/a.png");
    TEST_ASSERT_EQUAL_STRING("Assets/a.png", dest);
}

void test_platform_stringcopy_exact_fit(void)
{
    char dest[6];
    CP_StringCopy(dest, sizeof(dest), "abcde"); // 5 chars + NUL
    TEST_ASSERT_EQUAL_STRING("abcde", dest);
}

void test_platform_stringcopy_truncates_and_terminates(void)
{
    char dest[6];
    memset(dest, 'x', sizeof(dest));
    CP_StringCopy(dest, sizeof(dest), "abcdefghij");
    TEST_ASSERT_EQUAL_STRING("abcde", dest);
    TEST_ASSERT_EQUAL_CHAR('\0', dest[5]);
}

void test_platform_stringcopy_null_source_gives_empty_string(void)
{
    char dest[4] = { 'x', 'x', 'x', 'x' };
    CP_StringCopy(dest, sizeof(dest), NULL);
    TEST_ASSERT_EQUAL_STRING("", dest);
}

void test_platform_stringcopy_zero_size_writes_nothing(void)
{
    char dest[2] = { 'x', 'y' };
    CP_StringCopy(dest, 0, "abc");
    TEST_ASSERT_EQUAL_CHAR('x', dest[0]);
    TEST_ASSERT_EQUAL_CHAR('y', dest[1]);
    CP_StringCopy(NULL, 10, "abc"); // must not crash
}

void test_platform_path_buffer_is_large_enough(void)
{
#if defined(_WIN32)
    TEST_ASSERT_EQUAL_INT(260, CP_PATH_MAX); // historical MAX_PATH, unchanged
#else
    TEST_ASSERT_TRUE(CP_PATH_MAX >= 1024);   // POSIX paths can be much longer
#endif
}
