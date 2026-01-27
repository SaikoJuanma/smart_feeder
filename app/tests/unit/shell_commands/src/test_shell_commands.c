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
                  1,
                  "save_config should be called once, was called %d times",
                  save_config_fake.call_count);

    zassert_equal(last_saved_value, 999, "save_config called with wrong value. Expected 999, got %d", last_saved_value);
}

ZTEST(console_shell, test_value_cmd_set_negative)
{
    int ret = shell_execute_cmd(shell_backend, "value -50");
    zassert_equal(ret, 0, "Command execution failed");

    zassert_equal(cfg.random_value, -50, "Config not updated. Expected -50, got %d", cfg.random_value);

    zassert_equal(save_config_fake.call_count, 1, "save_config should be called once");
    zassert_equal(last_saved_value, -50, "save_config called with wrong value");
}

ZTEST(console_shell, test_value_cmd_set_zero)
{
    int ret = shell_execute_cmd(shell_backend, "value 0");
    zassert_equal(ret, 0, "Command execution failed");

    zassert_equal(cfg.random_value, 0, "Config not updated. Expected 0, got %d", cfg.random_value);

    zassert_equal(save_config_fake.call_count, 1, "save_config should be called once");
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

    zassert_equal(save_config_fake.call_count, 1, "save_config should be called even with invalid input");
}

ZTEST(console_shell, test_value_cmd_large_number)
{
    int ret = shell_execute_cmd(shell_backend, "value 2147483647");
    zassert_equal(ret, 0, "Command execution failed");

    zassert_equal(cfg.random_value, 2147483647, "Config not updated correctly");

    zassert_equal(save_config_fake.call_count, 1, "save_config should be called once");
    zassert_equal(last_saved_value, 2147483647, "save_config called with wrong value");
}

ZTEST(console_shell, test_value_cmd_multiple_saves)
{
    int ret = shell_execute_cmd(shell_backend, "value 100");
    zassert_equal(ret, 0, "First command failed");
    zassert_equal(save_config_fake.call_count, 1, "Expected 1 save call");

    shell_backend_dummy_clear_output(shell_backend);

    ret = shell_execute_cmd(shell_backend, "value 200");
    zassert_equal(ret, 0, "Second command failed");
    zassert_equal(save_config_fake.call_count, 2, "Expected 2 save calls");
    zassert_equal(last_saved_value, 200, "Last save should be 200");

    shell_backend_dummy_clear_output(shell_backend);

    ret = shell_execute_cmd(shell_backend, "value 300");
    zassert_equal(ret, 0, "Third command failed");
    zassert_equal(save_config_fake.call_count, 3, "Expected 3 save calls");
    zassert_equal(last_saved_value, 300, "Last save should be 300");
}

ZTEST(console_shell, test_value_cmd_saves_config)
{
    int ret = shell_execute_cmd(shell_backend, "value 555");
    zassert_equal(ret, 0, "Command execution failed");

    zassert_equal(save_config_fake.call_count,
                  1,
                  "save_config should be called exactly once, was called %d times",
                  save_config_fake.call_count);

    zassert_equal(last_saved_value, 555, "Expected saved value 555, got %d", last_saved_value);

    zassert_equal(cfg.random_value, 555, "Config should be updated to 555 before save");
}

ZTEST(console_shell_reboot, test_reboot_cmd_output)
{
    shell_execute_cmd(shell_backend, "reboot");

    zassert_unreachable("sys_reboot was not called!");
}

ZTEST_SUITE(console_shell, NULL, console_shell_setup, console_shell_before, console_shell_after, NULL);
/* INFO: this should run last, because it has a destructive effect */
ZTEST_SUITE(console_shell_reboot, NULL, console_shell_setup, console_shell_before, console_shell_after, NULL);
