#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/watchdog.h>
#include "init.h"
#include "motor_control.h"
#include "check_health.h"
#include "watchdog.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    /* INFO: start all the hardware related stuff */
    processes_init();

    /*INFO: start all the threads of the system */
    start_motor_control_thread();
    start_check_health_thread();

    while (1) {
        if (is_system_healthy()) {
            watchdog_feed();
        } else {
            LOG_WRN("System unhealthy");
        }

        k_sleep(K_MSEC(SUPERVISOR_CHECK_INTERVAL_MS));
    }

    return 0;
}
