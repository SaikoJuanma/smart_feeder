#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <zephyr/fff.h>
#include "check_health.h"

/* 1. Define FFF Globals */
DEFINE_FFF_GLOBALS;

/* 2. Define the Fake */
/* The linker is looking for 'z_impl_k_thread_stack_space_get'.
 * Signature: int z_impl_k_thread_stack_space_get(const struct k_thread *thread, size_t *unused_ptr)
 */
FAKE_VALUE_FUNC(int, z_impl_k_thread_stack_space_get, const struct k_thread *, size_t *);

/* Optional: Custom fake to set the output parameter 'unused_ptr' to avoid printing garbage logs */
int custom_stack_get_fake(const struct k_thread *thread, size_t *unused_ptr)
{
    if (unused_ptr) {
        *unused_ptr = 256; // Dummy value for the log
    }
    return 0;
}

static void check_health_tests_before(void *fixture)
{
    ARG_UNUSED(fixture);

    /* Reset the fake and assign custom behavior */
    RESET_FAKE(z_impl_k_thread_stack_space_get);
    z_impl_k_thread_stack_space_get_fake.custom_fake = custom_stack_get_fake;

    start_check_health_thread();

    /* Allow thread to run once before test starts */
    k_sleep(K_MSEC(HEALTH_CHECK_INTERVAL_MS + 10));
}

static void check_health_tests_after(void *fixture)
{
    ARG_UNUSED(fixture);

    stop_check_health_thread();
}

static void report_all_threads_alive(void)
{
    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_report_alive((thread_id_t)i);
    }
}

ZTEST(check_health, test_system_healthy_when_all_threads_report)
{
    report_all_threads_alive();

    k_sleep(K_MSEC(HEALTH_CHECK_INTERVAL_MS + 10));

    zassert_true(is_system_healthy(), "system unhealthy even though all threads reported alive");
}

ZTEST(check_health, test_system_unhealthy_when_no_heartbeat)
{
    k_sleep(K_MSEC(THREAD_TIMEOUT_MS + 10));

    zassert_false(is_system_healthy(), "system healthy despite no heartbeats");
}

ZTEST(check_health, test_system_unhealthy_when_one_thread_missing)
{
    for (int i = 0; i < THREAD_COUNT - 1; i++) {
        thread_report_alive((thread_id_t)i);
    }

    k_sleep(K_MSEC(THREAD_TIMEOUT_MS + 10));

    zassert_false(is_system_healthy(), "system healthy despite one missing heartbeat");
}

ZTEST(check_health, test_invalid_thread_id_does_not_crash)
{
    thread_report_alive((thread_id_t)THREAD_COUNT);

    zassert_true(true, "invalid thread id caused failure");
}

ZTEST_SUITE(check_health, NULL, NULL, check_health_tests_before, check_health_tests_after, NULL);
