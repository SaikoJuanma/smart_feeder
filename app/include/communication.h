#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#define COMMUNICATION_STACK    512
#define COMMUNICATION_PRIORITY 4

/**
 * @brief: starts the communication thread
 */
void start_comm_thread(void);

#ifdef SMART_FEEDER_UNIT_TEST
/**
 * @brief: Stops the motor control thread
 */
void stop_comm_thread(void);
#endif

#endif
