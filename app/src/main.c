#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "init.h"
#include "motor_control.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// TODO: we need a watchdog reset for the system.

int main(void)
{
    processes_init();
    start_motor_control_thread();

    while (1) {
        k_sleep(K_MSEC(20));
    }

    return 0;
}
