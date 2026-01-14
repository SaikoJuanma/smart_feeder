/**
 * @file: shell_commands.c
 * @brief: Shell command handlers.
 *
 * Registers shell commands used by the app.
 */
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(console_shell, LOG_LEVEL_INF);

/**
 * @brief: relevant info from the driver.
 *
 * Usage:
 *   status
 */
static int cmd_status(const struct shell *shell, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(shell, "this should print relevant info\n");

    return 0;
}

/* Register shell commands */
SHELL_CMD_REGISTER(status, NULL, "Print relevant info", cmd_status);
