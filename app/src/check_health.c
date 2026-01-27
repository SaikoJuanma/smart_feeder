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
static struct k_thread health_thread_data;
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
        k_sleep(K_SECONDS(5));
    }
}

void start_check_health_thread(void)
{
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
