/**
 * @file: check_healt.c
 * @brief: Monitoring of the system.
 *
 * Here we have all the monitoring of the non time critical stuff, like battery, temp, etc.
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "check_health.h"

LOG_MODULE_REGISTER(check_health, LOG_LEVEL_INF);

#define CHECK_HEALTH_STACK    512
#define CHECK_HEALTH_PRIORITY 5

K_THREAD_STACK_DEFINE(health_stack_area, CHECK_HEALTH_STACK);

/* Local prototypes */
static bool check_threads_health(void);

static struct k_thread health_thread_data;
static struct {
    uint32_t last_heartbeat[THREAD_COUNT];
    struct k_spinlock lock;
    bool threads_ok;
    // TODO: here we will add other things like battery, temperature, etc
} health_status;

static k_tid_t health_tid = NULL;

/**
 * @brief: Thread that checks general system info
 */
void check_health_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    size_t unused_stack;

    LOG_INF("Check health started at priority: %d", CHECK_HEALTH_PRIORITY);

    while (1) {
        k_thread_stack_space_get(&health_thread_data, &unused_stack);
        LOG_INF("Check health loop. Unused stack: %d bytes", unused_stack);

        check_threads_health();
        k_sleep(K_MSEC(HEALTH_CHECK_INTERVAL_MS));
    }
}

void start_check_health_thread(void)
{
    uint32_t now = k_uptime_get_32();

    for (int i = 0; i < THREAD_COUNT; i++) {
        health_status.last_heartbeat[i] = now;
    }
    health_status.threads_ok = true;

    health_tid = k_thread_create(&health_thread_data,
                                 health_stack_area,
                                 K_THREAD_STACK_SIZEOF(health_stack_area),
                                 check_health_thread,
                                 NULL,
                                 NULL,
                                 NULL,
                                 CHECK_HEALTH_PRIORITY,
                                 0,
                                 K_NO_WAIT);

    LOG_INF("Check health thread started (tid=%p)", (void *)health_tid);
}

// TODO: Unit tests
void thread_report_alive(thread_id_t thread_id)
{
    uint32_t now;

    if (thread_id >= THREAD_COUNT) {
        return;
    }

    now = k_uptime_get_32();
    k_spinlock_key_t key = k_spin_lock(&health_status.lock);
    health_status.last_heartbeat[thread_id] = now;
    k_spin_unlock(&health_status.lock, key);
}

/**
 * @brief: checks the health of all the threads
 * @return: true if all threads are ok, false otherwise
 */
static bool check_threads_health(void)
{
    uint32_t now = k_uptime_get_32();
    bool all_ok = true;

    for (int i = 0; i < THREAD_COUNT; i++) {
        uint32_t last_hb;
        uint32_t elapsed;

        k_spinlock_key_t key = k_spin_lock(&health_status.lock);
        last_hb = health_status.last_heartbeat[i];
        k_spin_unlock(&health_status.lock, key);

        elapsed = now - last_hb;
        if (elapsed > THREAD_TIMEOUT_MS) {
            LOG_INF("Thread %d not responding (last seen %u ms ago)", i, elapsed);
            all_ok = false;
        }
    }

    k_spinlock_key_t key = k_spin_lock(&health_status.lock);
    health_status.threads_ok = all_ok;
    k_spin_unlock(&health_status.lock, key);

    return all_ok;
}

bool is_system_healthy(void)
{
    bool healthy;
    k_spinlock_key_t key = k_spin_lock(&health_status.lock);
    // TODO:  we should add ands to check everything, like battery health
    healthy = health_status.threads_ok;
    k_spin_unlock(&health_status.lock, key);

    return healthy;
}

#ifdef SMART_FEEDER_UNIT_TEST
void stop_check_health_thread(void)
{
    if (health_tid != NULL) {
        LOG_INF("Stopping check health thread");
        k_thread_abort(health_tid);
        health_tid = NULL;
    }
}
#endif
