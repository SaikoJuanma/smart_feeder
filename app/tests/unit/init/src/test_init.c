#include <zephyr/ztest.h>
#include <zephyr/fff.h>
#include "init.h"
#include "configuration.h"

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(int, init_nvs);
FAKE_VALUE_FUNC(int, load_config);
FAKE_VOID_FUNC(set_dflt_cfg);
FAKE_VALUE_FUNC(int, init_watchdog);

static void init_tests_before(void *fixture)
{
    RESET_FAKE(init_nvs);
    RESET_FAKE(load_config);
    RESET_FAKE(set_dflt_cfg);
    RESET_FAKE(init_watchdog);
    FFF_RESET_HISTORY();

    init_nvs_fake.return_val = 0;
    load_config_fake.return_val = 0;
    init_watchdog_fake.return_val = 0;
}

ZTEST(init_tests, test_processes_init_success_with_config)
{
    int ret = processes_init();

    zassert_equal(ret, 0, NULL);
    zassert_equal(init_nvs_fake.call_count, 1, NULL);
    zassert_equal(load_config_fake.call_count, 1, NULL);
    zassert_equal(set_dflt_cfg_fake.call_count, 0, NULL);
    zassert_equal(init_watchdog_fake.call_count, 1, NULL);
}

ZTEST(init_tests, test_processes_init_success_with_defaults)
{
    load_config_fake.return_val = -1;

    int ret = processes_init();

    zassert_equal(ret, 0, NULL);
    zassert_equal(init_nvs_fake.call_count, 1, NULL);
    zassert_equal(load_config_fake.call_count, 1, NULL);
    zassert_equal(set_dflt_cfg_fake.call_count, 1, NULL);
    zassert_equal(init_watchdog_fake.call_count, 1, NULL);
}

ZTEST(init_tests, test_processes_init_nvs_failure)
{
    init_nvs_fake.return_val = -5;

    int ret = processes_init();

    zassert_equal(ret, -5, NULL);
    zassert_equal(load_config_fake.call_count, 0, NULL);
    zassert_equal(set_dflt_cfg_fake.call_count, 0, NULL);
    zassert_equal(init_watchdog_fake.call_count, 0, NULL);
}

ZTEST(init_tests, test_processes_init_watchdog_failure)
{
    init_watchdog_fake.return_val = -EINVAL;

    int ret = processes_init();

    zassert_equal(ret, -EINVAL, NULL);
    zassert_equal(init_watchdog_fake.call_count, 1, NULL);
}

ZTEST_SUITE(init_tests, NULL, NULL, init_tests_before, NULL, NULL);
