/**
 * @file: motor_control.c
 * @brief: Motor control functions.
 *
 * All the functions directly related to the control of the stepper motor should be here.
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "motor_control.h"
#include "check_health.h"

LOG_MODULE_REGISTER(motor_control, LOG_LEVEL_INF);

#define MOTOR_CTRL_STACK    512
#define MOTOR_CTRL_PRIORITY 1

K_THREAD_STACK_DEFINE(motor_stack_area, MOTOR_CTRL_STACK);
static struct k_thread motor_thread_data;
static k_tid_t motor_tid = NULL;

/**
 * @brief: Thread that is in charge of controlling the stepper motor
 */
void motor_control_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);
    size_t unused_stack;

    LOG_INF("Motor control started at priority: %d", MOTOR_CTRL_PRIORITY);

    while (1) {
        thread_report_alive(THREAD_MOTOR_CONTROL);
        k_thread_stack_space_get(&motor_thread_data, &unused_stack);
        LOG_INF("Motor control loop. Unused Stack: %d bytes", unused_stack);
        k_sleep(K_MSEC(500));
    }
}

void start_motor_control_thread(void)
{
    motor_tid = k_thread_create(&motor_thread_data,
                                motor_stack_area,
                                K_THREAD_STACK_SIZEOF(motor_stack_area),
                                motor_control_thread,
                                NULL,
                                NULL,
                                NULL,
                                MOTOR_CTRL_PRIORITY,
                                0,
                                K_NO_WAIT);

    LOG_INF("Motor control thread started (tid=%p)", (void *)motor_tid);
}

#ifdef SMART_FEEDER_UNIT_TEST
void stop_motor_control_thread(void)
{
    if (motor_tid != NULL) {
        LOG_INF("Stopping motor control thread");
        k_thread_abort(motor_tid);
        motor_tid = NULL;
    }
}
#endif
