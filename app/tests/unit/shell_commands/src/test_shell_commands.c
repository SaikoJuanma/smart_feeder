#include <zephyr/ztest.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_dummy.h>
#include <zephyr/fff.h>
#include <zephyr/sys/reboot.h>
#include <string.h>
#include "configuration.h"

DEFINE_FFF_GLOBALS;

static const struct shell *shell_backend;

struct config cfg;
static struct config backup_cfg;

FAKE_VALUE_FUNC(int, save_config);
FAKE_VOID_FUNC(set_dflt_cfg);

struct sys_reboot_fake_context {
    int call_count;
    int arg0_val;
};

struct sys_reboot_fake_context sys_reboot_fake;

void sys_reboot(int type)
{
    sys_reboot_fake.call_count++;
    sys_reboot_fake.arg0_val = type;

    zassert_equal(sys_reboot_fake.call_count, 1, "sys_reboot called more than once");

    ztest_test_pass();

    while (1) {
        k_sleep(K_FOREVER);
    }
}

static int last_saved_value = 0;

int custom_save_config(void)
{
    last_saved_value = cfg.random_value;
    return 0;
}

static void *console_shell_setup(void)
{
    shell_backend = shell_backend_dummy_get_ptr();
    zassert_not_null(shell_backend, "Failed to get shell backend");
    return NULL;
}

static void console_shell_before(void *fixture)
{
    ARG_UNUSED(fixture);

    memcpy(&backup_cfg, &cfg, sizeof(struct config));

    RESET_FAKE(save_config);
    RESET_FAKE(set_dflt_cfg);

    sys_reboot_fake.call_count = 0;
    sys_reboot_fake.arg0_val = 0;

    FFF_RESET_HISTORY();

    save_config_fake.custom_fake = custom_save_config;
    last_saved_value = 0;

    shell_backend_dummy_clear_output(shell_backend);

    k_msleep(100);
}

static void console_shell_after(void *fixture)
{
    ARG_UNUSED(fixture);

    memcpy(&cfg, &backup_cfg, sizeof(struct config));
}

ZTEST(console_shell, test_value_cmd_no_args_no_save)
{
    cfg.random_value = 123;

    int ret = shell_execute_cmd(shell_backend, "value");
    zassert_equal(ret, 0, "Command execution failed");

    zassert_equal(save_config_fake.call_count, 0, "save_config should not be called when just displaying value");
}

ZTEST(console_shell, test_status_cmd_output)
{
    size_t output_len;
    cfg.random_value = 42;

    int ret = shell_execute_cmd(shell_backend, "status");
    zassert_equal(ret, 0, "Command execution failed");

    const char *output = shell_backend_dummy_get_output(shell_backend, &output_len);
    zassert_not_null(output, "No output captured");

    zassert_true(strstr(output, "Current config:") != NULL, "Expected 'Current config:' in output. Got: '%s'", output);
    zassert_true(strstr(output, "42") != NULL, "Expected '42' in output. Got: '%s'", output);
}

ZTEST(console_shell, test_value_cmd_no_args)
{
    size_t output_len;
    cfg.random_value = 123;

    int ret = shell_execute_cmd(shell_backend, "value");
    zassert_equal(ret, 0, "Command execution failed");

    const char *output = shell_backend_dummy_get_output(shell_backend, &output_len);
    zassert_not_null(output, "No output captured");
    zassert_true(strstr(output, "Random value:") != NULL, "Expected 'Random value:' in output. Got: '%s'", output);
    zassert_true(strstr(output, "123") != NULL, "Expected '123' in output. Got: '%s'", output);
}

ZTEST(console_shell, test_value_cmd_set_positive)
{
    size_t output_len;

    int ret = shell_execute_cmd(shell_backend, "value 999");
    zassert_equal(ret, 0, "Command execution failed");

    const char *output = shell_backend_dummy_get_output(shell_backend, &output_len);
    zassert_not_null(output, "No output captured");
    zassert_true(strstr(output, "changing value to:") != NULL, "Expected confirmation message. Got: '%s'", output);
    zassert_true(strstr(output, "999") != NULL, "Expected '999' in output. Got: '%s'", output);

    zassert_equal(cfg.random_value, 999, "Config not updated. Expected 999, got %d", cfg.random_value);

    zassert_equal(save_config_fake.call_count,
                  0,
                  "save_config should NOT be called (removed from value command), was called %d times",
                  save_config_fake.call_count);
}

ZTEST(console_shell, test_value_cmd_set_negative)
{
    int ret = shell_execute_cmd(shell_backend, "value -50");
    zassert_equal(ret, 0, "Command execution failed");

    zassert_equal(cfg.random_value, -50, "Config not updated. Expected -50, got %d", cfg.random_value);

    zassert_equal(save_config_fake.call_count, 0, "save_config should NOT be called");
}

ZTEST(console_shell, test_value_cmd_set_zero)
{
    int ret = shell_execute_cmd(shell_backend, "value 0");
    zassert_equal(ret, 0, "Command execution failed");

    zassert_equal(cfg.random_value, 0, "Config not updated. Expected 0, got %d", cfg.random_value);

    zassert_equal(save_config_fake.call_count, 0, "save_config should NOT be called");
}

ZTEST(console_shell, test_value_cmd_too_many_args)
{
    size_t output_len;

    int ret = shell_execute_cmd(shell_backend, "value 100 200");
    zassert_equal(ret, -EINVAL, "Expected EINVAL, got %d", ret);

    const char *output = shell_backend_dummy_get_output(shell_backend, &output_len);
    zassert_not_null(output, "No output captured");
    zassert_true(strstr(output, "Usage:") != NULL, "Expected usage message. Got: '%s'", output);

    zassert_equal(save_config_fake.call_count, 0, "save_config should not be called on error");
}

ZTEST(console_shell, test_value_cmd_invalid_arg)
{
    int ret = shell_execute_cmd(shell_backend, "value abc");
    zassert_equal(ret, 0, "Command should execute (atoi behavior)");

    zassert_equal(cfg.random_value, 0, "Expected 0 (atoi behavior), got %d", cfg.random_value);

    zassert_equal(save_config_fake.call_count, 0, "save_config should NOT be called");
}

ZTEST(console_shell, test_value_cmd_large_number)
{
    int ret = shell_execute_cmd(shell_backend, "value 2147483647");
    zassert_equal(ret, 0, "Command execution failed");

    zassert_equal(cfg.random_value, 2147483647, "Config not updated correctly");

    zassert_equal(save_config_fake.call_count, 0, "save_config should NOT be called");
}

ZTEST(console_shell, test_value_cmd_multiple_changes)
{
    int ret = shell_execute_cmd(shell_backend, "value 100");
    zassert_equal(ret, 0, "First command failed");
    zassert_equal(save_config_fake.call_count, 0, "save_config should NOT be called");
    zassert_equal(cfg.random_value, 100, "Config should be 100");

    shell_backend_dummy_clear_output(shell_backend);

    ret = shell_execute_cmd(shell_backend, "value 200");
    zassert_equal(ret, 0, "Second command failed");
    zassert_equal(save_config_fake.call_count, 0, "save_config should NOT be called");
    zassert_equal(cfg.random_value, 200, "Config should be 200");

    shell_backend_dummy_clear_output(shell_backend);

    ret = shell_execute_cmd(shell_backend, "value 300");
    zassert_equal(ret, 0, "Third command failed");
    zassert_equal(save_config_fake.call_count, 0, "save_config should NOT be called");
    zassert_equal(cfg.random_value, 300, "Config should be 300");
}

ZTEST(console_shell, test_value_cmd_does_not_save_config)
{
    int ret = shell_execute_cmd(shell_backend, "value 555");
    zassert_equal(ret, 0, "Command execution failed");

    zassert_equal(save_config_fake.call_count,
                  0,
                  "save_config should NOT be called by value command, was called %d times",
                  save_config_fake.call_count);

    zassert_equal(cfg.random_value, 555, "Config should be updated to 555");
}

/* ========== COMMIT COMMAND TESTS ========== */

ZTEST(console_shell, test_commit_cmd_calls_save_config)
{
    cfg.random_value = 777;

    int ret = shell_execute_cmd(shell_backend, "commit");
    zassert_equal(ret, 0, "Command execution failed");

    zassert_equal(save_config_fake.call_count,
                  1,
                  "save_config should be called exactly once, was called %d times",
                  save_config_fake.call_count);
    zassert_equal(last_saved_value, 777, "Expected saved value 777, got %d", last_saved_value);
}

ZTEST(console_shell, test_commit_cmd_output)
{
    size_t output_len;

    int ret = shell_execute_cmd(shell_backend, "commit");
    zassert_equal(ret, 0, "Command execution failed");

    const char *output = shell_backend_dummy_get_output(shell_backend, &output_len);
    zassert_not_null(output, "No output captured");
    zassert_true(strstr(output, "Config saved in the NVS") != NULL,
                 "Expected 'Config saved in the NVS' in output. Got: '%s'",
                 output);
}

ZTEST(console_shell, test_commit_cmd_with_args_ignored)
{
    cfg.random_value = 888;

    int ret = shell_execute_cmd(shell_backend, "commit extra args");
    zassert_equal(ret, 0, "Command execution failed");

    zassert_equal(save_config_fake.call_count, 1, "save_config should be called once");
    zassert_equal(last_saved_value, 888, "Expected saved value 888, got %d", last_saved_value);
}

ZTEST(console_shell, test_commit_multiple_times)
{
    cfg.random_value = 100;
    int ret = shell_execute_cmd(shell_backend, "commit");
    zassert_equal(ret, 0, "First commit failed");
    zassert_equal(save_config_fake.call_count, 1, "Expected 1 save call");
    zassert_equal(last_saved_value, 100, "Expected saved value 100");

    shell_backend_dummy_clear_output(shell_backend);

    cfg.random_value = 200;
    ret = shell_execute_cmd(shell_backend, "commit");
    zassert_equal(ret, 0, "Second commit failed");
    zassert_equal(save_config_fake.call_count, 2, "Expected 2 save calls");
    zassert_equal(last_saved_value, 200, "Expected saved value 200");
}

ZTEST(console_shell, test_value_then_commit_workflow)
{
    cfg.random_value = 50;

    int ret = shell_execute_cmd(shell_backend, "value 150");
    zassert_equal(ret, 0, "value command failed");
    zassert_equal(cfg.random_value, 150, "Config should be 150");
    zassert_equal(save_config_fake.call_count, 0, "save_config should not be called by value");

    shell_backend_dummy_clear_output(shell_backend);

    ret = shell_execute_cmd(shell_backend, "commit");
    zassert_equal(ret, 0, "commit command failed");
    zassert_equal(save_config_fake.call_count, 1, "save_config should be called by commit");
    zassert_equal(last_saved_value, 150, "Expected saved value 150");
}

/* ========== DEFAULT COMMAND TESTS ========== */

ZTEST(console_shell, test_default_cmd_calls_set_dflt_cfg)
{
    int ret = shell_execute_cmd(shell_backend, "default");
    zassert_equal(ret, 0, "Command execution failed");

    zassert_equal(set_dflt_cfg_fake.call_count,
                  1,
                  "set_dflt_cfg should be called exactly once, was called %d times",
                  set_dflt_cfg_fake.call_count);
}

ZTEST(console_shell, test_default_cmd_output)
{
    size_t output_len;

    int ret = shell_execute_cmd(shell_backend, "default");
    zassert_equal(ret, 0, "Command execution failed");

    const char *output = shell_backend_dummy_get_output(shell_backend, &output_len);
    zassert_not_null(output, "No output captured");
    zassert_true(strstr(output, "Default values restored") != NULL,
                 "Expected 'Default values restored' in output. Got: '%s'",
                 output);
}

ZTEST(console_shell, test_default_cmd_with_args_ignored)
{
    int ret = shell_execute_cmd(shell_backend, "default extra args");
    zassert_equal(ret, 0, "Command execution failed");

    zassert_equal(set_dflt_cfg_fake.call_count, 1, "set_dflt_cfg should be called once");
}

ZTEST(console_shell, test_default_multiple_times)
{
    int ret = shell_execute_cmd(shell_backend, "default");
    zassert_equal(ret, 0, "First default failed");
    zassert_equal(set_dflt_cfg_fake.call_count, 1, "Expected 1 call");

    shell_backend_dummy_clear_output(shell_backend);

    ret = shell_execute_cmd(shell_backend, "default");
    zassert_equal(ret, 0, "Second default failed");
    zassert_equal(set_dflt_cfg_fake.call_count, 2, "Expected 2 calls");
}

ZTEST(console_shell, test_default_does_not_save_config)
{
    int ret = shell_execute_cmd(shell_backend, "default");
    zassert_equal(ret, 0, "Command execution failed");

    zassert_equal(set_dflt_cfg_fake.call_count, 1, "set_dflt_cfg should be called");
    zassert_equal(save_config_fake.call_count, 0, "save_config should NOT be called automatically by default command");
}

ZTEST(console_shell, test_default_then_commit_workflow)
{
    cfg.random_value = 999;

    int ret = shell_execute_cmd(shell_backend, "default");
    zassert_equal(ret, 0, "default command failed");
    zassert_equal(set_dflt_cfg_fake.call_count, 1, "set_dflt_cfg should be called");
    zassert_equal(save_config_fake.call_count, 0, "save_config should not be called by default");

    shell_backend_dummy_clear_output(shell_backend);

    ret = shell_execute_cmd(shell_backend, "commit");
    zassert_equal(ret, 0, "commit command failed");
    zassert_equal(save_config_fake.call_count, 1, "save_config should be called by commit");
}

ZTEST(console_shell, test_value_default_commit_workflow)
{
    cfg.random_value = 0;

    int ret = shell_execute_cmd(shell_backend, "value 500");
    zassert_equal(ret, 0, "value command failed");
    zassert_equal(cfg.random_value, 500, "Config should be 500");

    shell_backend_dummy_clear_output(shell_backend);

    ret = shell_execute_cmd(shell_backend, "default");
    zassert_equal(ret, 0, "default command failed");
    zassert_equal(set_dflt_cfg_fake.call_count, 1, "set_dflt_cfg should be called");

    shell_backend_dummy_clear_output(shell_backend);

    ret = shell_execute_cmd(shell_backend, "commit");
    zassert_equal(ret, 0, "commit command failed");
    zassert_equal(save_config_fake.call_count, 1, "save_config should be called");
}

/* ========== REBOOT TEST ========== */

ZTEST(console_shell_reboot, test_reboot_cmd_output)
{
    shell_execute_cmd(shell_backend, "reboot");

    zassert_unreachable("sys_reboot was not called!");
}

ZTEST_SUITE(console_shell, NULL, console_shell_setup, console_shell_before, console_shell_after, NULL);
/* INFO: this should run last, because it has a destructive effect */
ZTEST_SUITE(console_shell_reboot, NULL, console_shell_setup, console_shell_before, console_shell_after, NULL);
