#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "init.h"
#include "motor_control.h"
#include "check_health.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// TODO: we need a watchdog reset for the system.
// is the watchdog apropriate on the main loop?

int main(void)
{
    /* INFO: start all the hardware related stuff */
    processes_init();

    /*INFO: start all the threads of the system */
    start_motor_control_thread();
    start_check_health_thread();

    while (1) {
        k_sleep(K_MSEC(20));
    }

    return 0;
}
