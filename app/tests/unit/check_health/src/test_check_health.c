#include <zephyr/ztest.h>

ZTEST(my_tests, test_placeholder)
{
    zassert_true(true, "Placeholder test");
}

ZTEST_SUITE(my_tests, NULL, NULL, NULL, NULL, NULL);
