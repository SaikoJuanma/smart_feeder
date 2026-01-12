#include <zephyr/ztest.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_dummy.h>
#include <string.h>

static const struct shell *shell_backend;

static void *console_shell_setup(void)
{
    shell_backend = shell_backend_dummy_get_ptr();
    zassert_not_null(shell_backend, "Failed to get shell backend");
    return NULL;
}

static void console_shell_before(void *fixture)
{
    /* INFO: Clear output before each test */
    shell_backend_dummy_clear_output(shell_backend);
    /* INFO: Give shell commands time to register */
    k_msleep(100);
}

ZTEST(console_shell, test_status_cmd)
{
    const char *expected_output = "this should print relevant info";
    size_t output_len;

    int ret = shell_execute_cmd(shell_backend, "status");
    zassert_equal(ret, 0, "Command execution failed with code: %d", ret);

    const char *output = shell_backend_dummy_get_output(shell_backend, &output_len);

    printk("Output length: %zu\n", output_len);
    if (output) {
        printk("Output: '%s'\n", output);
    }

    zassert_not_null(output, "No output captured");
    zassert_true(strstr(output, expected_output) != NULL, "Expected output not found. Got: '%s'", output);
}

ZTEST_SUITE(console_shell, NULL, console_shell_setup, console_shell_before, NULL, NULL);
