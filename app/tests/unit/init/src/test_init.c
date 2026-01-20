#include <zephyr/ztest.h>
#include <zephyr/fff.h>
#include "init.h"
#include "configuration.h"

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(int, init_nvs);
FAKE_VALUE_FUNC(int, load_config);
FAKE_VOID_FUNC(set_dflt_cfg);

static void init_tests_before(void *fixture)
{
    RESET_FAKE(init_nvs);
    RESET_FAKE(load_config);
    RESET_FAKE(set_dflt_cfg);
    FFF_RESET_HISTORY();
}

ZTEST(init_tests, test_processes_init_success_with_config)
{
    init_nvs_fake.return_val = 0;
    load_config_fake.return_val = 0;

    int ret = processes_init();

    zassert_equal(ret, 0, "Expected successful initialization, got: %d", ret);
    zassert_equal(init_nvs_fake.call_count, 1, "init_nvs should be called once, called: %d", init_nvs_fake.call_count);
    zassert_equal(
        load_config_fake.call_count, 1, "load_config should be called once, called: %d", load_config_fake.call_count);
    zassert_equal(
        set_dflt_cfg_fake.call_count, 0, "set_dflt_cfg should not be called, called: %d", set_dflt_cfg_fake.call_count);
}

ZTEST(init_tests, test_processes_init_success_with_defaults)
{
    init_nvs_fake.return_val = 0;
    load_config_fake.return_val = -1;

    int ret = processes_init();

    zassert_equal(ret, 0, "Expected successful initialization, got: %d", ret);
    zassert_equal(init_nvs_fake.call_count, 1, "init_nvs should be called once, called: %d", init_nvs_fake.call_count);
    zassert_equal(
        load_config_fake.call_count, 1, "load_config should be called once, called: %d", load_config_fake.call_count);
    zassert_equal(set_dflt_cfg_fake.call_count,
                  1,
                  "set_dflt_cfg should be called once, called: %d",
                  set_dflt_cfg_fake.call_count);
}

ZTEST(init_tests, test_processes_init_nvs_failure)
{
    init_nvs_fake.return_val = -5;

    int ret = processes_init();

    zassert_equal(ret, -5, "Expected NVS error code to be returned, got: %d", ret);
    zassert_equal(init_nvs_fake.call_count, 1, "init_nvs should be called once, called: %d", init_nvs_fake.call_count);
    zassert_equal(load_config_fake.call_count,
                  0,
                  "load_config should not be called after NVS failure called: %d",
                  load_config_fake.call_count);
    zassert_equal(set_dflt_cfg_fake.call_count,
                  0,
                  "set_dflt_cfg should not be called after NVS failure, called: %d",
                  set_dflt_cfg_fake.call_count);
}

ZTEST(init_tests, test_processes_init_various_nvs_errors)
{
    int error_codes[] = {-1, -10, -100};

    for (int i = 0; i < ARRAY_SIZE(error_codes); i++) {
        RESET_FAKE(init_nvs);
        RESET_FAKE(load_config);
        RESET_FAKE(set_dflt_cfg);

        init_nvs_fake.return_val = error_codes[i];

        int ret = processes_init();

        zassert_equal(ret, error_codes[i], "Expected error code %d to be returned, got %d", error_codes[i], ret);
    }
}

ZTEST(init_tests, test_processes_init_load_config_errors_use_defaults)
{
    int error_codes[] = {-1, -2, -10};

    for (int i = 0; i < ARRAY_SIZE(error_codes); i++) {
        RESET_FAKE(init_nvs);
        RESET_FAKE(load_config);
        RESET_FAKE(set_dflt_cfg);

        init_nvs_fake.return_val = 0;
        load_config_fake.return_val = error_codes[i];

        int ret = processes_init();

        zassert_equal(ret, 0, "Should succeed with defaults even if load_config fails, got: %d", ret);
        zassert_equal(set_dflt_cfg_fake.call_count,
                      1,
                      "set_dflt_cfg should be called when load_config returns %d, called: %d",
                      error_codes[i],
                      set_dflt_cfg_fake.call_count);
    }
}

ZTEST_SUITE(init_tests, NULL, NULL, init_tests_before, NULL, NULL);
