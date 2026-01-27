#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

/**
 * @brief: Starts the motor control thread
 */
void start_motor_control_thread(void);

#ifdef SMART_FEEDER_UNIT_TEST
/**
 * @brief: Stops the motor control thread
 */
void stop_motor_control_thread(void);
#endif
#endif
