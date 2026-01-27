#ifndef CHECK_HEALTH_H
#define CHECK_HEALTH_H

/**
 * @brief: Starts the check health thread
 */
void start_check_health_thread(void);

#ifdef SMART_FEEDER_UNIT_TEST
/**
 * @brief: Stops the motor control thread
 */
void stop_check_health_thread(void);
#endif

#endif
