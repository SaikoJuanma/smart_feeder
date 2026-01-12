#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_dummy.h>

ZTEST(smart_feeder_integration, test_system_initialization)
{
    zassert_true(k_uptime_get() > 0, "System should be running");

    k_msleep(100);
}

ZTEST(smart_feeder_integration, test_shell_command_execution)
{
    const struct shell *sh = shell_backend_dummy_get_ptr();
    int ret;

    zassert_not_null(sh, "Shell backend should be available");

    shell_backend_dummy_clear_output(sh);

    ret = shell_execute_cmd(sh, "status");
    zassert_equal(ret, 0, "Status command should execute successfully");

    k_msleep(50);
}

/* Placeholder for future module integration tests */
ZTEST(smart_feeder_integration, test_future_module_interaction)
{
    /* TODO: Add tests when motor control module is added */
    /* TODO: Add tests when sensor modules are added */
    /* TODO: Add tests when feeding schedule is implemented */

    k_msleep(100);
    zassert_true(true, "System remains stable");
}

ZTEST_SUITE(smart_feeder_integration, NULL, NULL, NULL, NULL, NULL);
