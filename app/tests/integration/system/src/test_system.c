#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_dummy.h>
#include <zephyr/logging/log.h>
#include "init.h"
#include "configuration.h"

LOG_MODULE_REGISTER(integration_test, LOG_LEVEL_INF);

static void *integration_setup(void)
{
    int ret = init_nvs();
    zassert_equal(ret, 0, "NVS initialization failed");

    processes_init();

    k_msleep(100);

    return NULL;
}

ZTEST_SUITE(smart_feeder_integration, NULL, integration_setup, NULL, NULL, NULL);

ZTEST(smart_feeder_integration, test_system_initialization)
{
    zassert_true(k_uptime_get() > 0, "System uptime should be positive");
    zassert_true(cfg.random_value >= 0, "Config value invalid");
}

ZTEST(smart_feeder_integration, test_shell_status_cmd)
{
    const struct shell *sh = shell_backend_dummy_get_ptr();
    size_t output_len;
    int ret;

    zassert_not_null(sh, "Shell backend not found");

    shell_backend_dummy_clear_output(sh);

    ret = shell_execute_cmd(sh, "status");
    zassert_equal(ret, 0, "Command 'status' failed");

    const char *output = shell_backend_dummy_get_output(sh, &output_len);
    zassert_not_null(output, "No output from status command");

    zassert_true(strstr(output, "Current config") != NULL, "Status output missing 'Current config'");
}

ZTEST(smart_feeder_integration, test_shell_value_update)
{
    const struct shell *sh = shell_backend_dummy_get_ptr();
    int ret;
    int test_val = 456;
    char cmd_buf[32];

    zassert_not_null(sh, "Shell backend not found");
    shell_backend_dummy_clear_output(sh);

    snprintf(cmd_buf, sizeof(cmd_buf), "value %d", test_val);
    ret = shell_execute_cmd(sh, cmd_buf);
    zassert_equal(ret, 0, "Command 'value' failed");

    zassert_equal(cfg.random_value, test_val, "Global config not updated immediately");

    k_msleep(50);

    cfg.random_value = 0;

    ret = load_config();
    zassert_equal(ret, 0, "Failed to reload config");

    zassert_equal(cfg.random_value, test_val, "Failed to persist value to NVS");
}
