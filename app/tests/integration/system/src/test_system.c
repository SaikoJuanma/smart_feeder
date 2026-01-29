#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include "init.h"
#include "motor_control.h"
#include "configuration.h"
#include "check_health.h"
#include "watchdog.h"
#include "communication.h"

ZTEST(smart_feeder_integration, test_system_threads)
{
    int ret = init_nvs();
    zassert_equal(ret, 0, "NVS initialization failed");

    processes_init();

    start_motor_control_thread();
    start_check_health_thread();
    start_comm_thread();

    for (int i = 0; i < 11; i++) {
        k_msleep(100);
        watchdog_feed();
    }

    watchdog_disable();

    zassert_true(k_uptime_get() > 0, "System should be running");
    zassert_true(is_system_healthy(), "System reported unhealthy state");

#ifdef SMART_FEEDER_UNIT_TEST
    stop_motor_control_thread();
    stop_check_health_thread();
    stop_comm_thread();
#endif
}

ZTEST_SUITE(smart_feeder_integration, NULL, NULL, NULL, NULL, NULL);
