/**
 * @file: communication.c
 * @brief: Thread that is in charge of the comms.
 *
 * In this file, is the main communication thread, that will take charge of interfacing with the pc/app or other nodes
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "communication.h"
#include "check_health.h"

LOG_MODULE_REGISTER(communication, LOG_LEVEL_INF);
K_THREAD_STACK_DEFINE(comm_stack_area, COMMUNICATION_STACK);

static struct k_thread communication_thread_data;

static k_tid_t comm_tid = NULL;

// TODO: should be always listenning and use work_queue when we need to transmit something
/**
 * @brief: starts the communication thread
 */
void comm_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    size_t unused_stack;

    LOG_INF("Comm thread started with priority: %d", COMMUNICATION_PRIORITY);

    while (1) {
        k_thread_stack_space_get(&communication_thread_data, &unused_stack);
        thread_report_alive(THREAD_COMMUNICATION);
        LOG_INF("Communication loop. Unused stack: %d bytes", unused_stack);
        k_sleep(K_MSEC(100));
    }
}

void start_comm_thread(void)
{
    comm_tid = k_thread_create(&communication_thread_data,
                               comm_stack_area,
                               K_THREAD_STACK_SIZEOF(comm_stack_area),
                               comm_thread,
                               NULL,
                               NULL,
                               NULL,
                               COMMUNICATION_PRIORITY,
                               0,
                               K_NO_WAIT);

    LOG_INF("Communication thread started (tid=%p)", (void *)comm_tid);
}

#ifdef SMART_FEEDER_UNIT_TEST
void stop_comm_thread(void)
{
    if (comm_tid != NULL) {
        LOG_INF("Stopping communication thread");
        k_thread_abort(comm_tid);
        comm_tid = NULL;
    }
}

#endif /* ifdef MACRO */
