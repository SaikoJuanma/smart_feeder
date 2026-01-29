#ifndef CHECK_HEALTH_H
#define CHECK_HEALTH_H

#include <stdint.h>
#include <stdbool.h>

#define CHECK_HEALTH_STACK       512
#define CHECK_HEALTH_PRIORITY    5
#define THREAD_TIMEOUT_MS        500
#define HEALTH_CHECK_INTERVAL_MS 100

typedef enum {
    THREAD_MOTOR_CONTROL = 0,
    THREAD_COMMUNICATION,
    THREAD_COUNT
} thread_id_t;

/**
 * @brief: Starts the check health thread
 */
void start_check_health_thread(void);

/**
 * @brief: Threads call this to report they're alive
 * @param: thread_id Which thread is reporting
 */
// TODO: unit test left to do
void thread_report_alive(thread_id_t thread_id);

/**
 * @brief: Main supervisor calls this to check overall system health
 * @return: true if all monitored systems are healthy, false otherwise
 */
// TODO: unit tests left
bool is_system_healthy(void);

#ifdef SMART_FEEDER_UNIT_TEST
/**
 * @brief: Stops the motor control thread
 */
void stop_check_health_thread(void);
#endif

#endif
