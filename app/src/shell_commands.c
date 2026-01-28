/**
 * @file: shell_commands.c
 * @brief: Shell command handlers.
 *
 * Registers shell commands used by the app.
 */
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>
#include <stdlib.h>
#include "configuration.h"

// TODO: commit command
// TODO: restore dflt command

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

    shell_print(shell, "Current config: %d", cfg.random_value);

    return 0;
}

/**
 * @brief: Changes the value and saves it to the nvs.
 *
 * Usage:
 *   value <int>
 */
static int cmd_change_value(const struct shell *shell, size_t argc, char **argv)
{
    if (argc == 1) {
        shell_print(shell, "Random value: %d", cfg.random_value);
        return 0;
    } else if (argc == 2) {
        cfg.random_value = atoi(argv[1]);
        shell_print(shell, "changing value to: %d", cfg.random_value);
        return 0;
    }

    shell_print(shell, "Usage: value <int>");
    return -EINVAL;
}

/**
 * @brief: Cold reboots the system
 *
 * Usage:
 *     reboot
 */
static int cmd_reboot(const struct shell *shell, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(shell, "Rebooting the system . . .");
    sys_reboot(SYS_REBOOT_COLD);
    return 0;
}

/**
 * @brief: Saves the current config in the nvs
 *
 * Usage:
 *     commit
 */
static int cmd_commit(const struct shell *shell, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    save_config();
    shell_print(shell, "Config saved in the NVS");
    return 0;
}

/**
 * @brief: Restores the default config
 *
 * Usage:
 *     default
 */
static int cmd_restore_dflt(const struct shell *shell, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    set_dflt_cfg();
    shell_print(shell, "Default values restored");
    return 0;
}

/* Register shell commands */
SHELL_CMD_REGISTER(status, NULL, "Print relevant info", cmd_status);
SHELL_CMD_REGISTER(value, NULL, "Change the random value", cmd_change_value);
SHELL_CMD_REGISTER(reboot, NULL, "Colds reboots the system", cmd_reboot);
SHELL_CMD_REGISTER(commit, NULL, "Saves the current config in the NVS", cmd_commit);
SHELL_CMD_REGISTER(default, NULL, "Restores the default values", cmd_restore_dflt);
