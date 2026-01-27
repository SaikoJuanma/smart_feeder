#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include "init.h"
#include "motor_control.h"
#include "configuration.h"
#include "check_health.h"

ZTEST(smart_feeder_integration, test_system_threads)
{
    int ret = init_nvs();
    zassert_equal(ret, 0, "NVS initialization failed");

    processes_init();

    start_motor_control_thread();
    start_check_health_thread();

    k_msleep(1100);

    zassert_true(k_uptime_get() > 0, "System should be running");

    /* TODO: When motor logic exists, exercise the thread:
     * - Send motor commands
     * - Verify state changes
     * - Test different operating conditions
     */

    /* INFO: Stop test-only (prevents thread from affecting other tests) */
#ifdef SMART_FEEDER_UNIT_TEST
    stop_motor_control_thread();
    stop_check_health_thread();
#endif
}

ZTEST_SUITE(smart_feeder_integration, NULL, NULL, NULL, NULL, NULL);
