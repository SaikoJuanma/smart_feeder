#ifndef WATCHDOG_H
#define WATCHDOG_H

#define WDT_TIMEOUT_MS               1000
#define SUPERVISOR_CHECK_INTERVAL_MS 500

/**
 * @brief: feeds the watchdog so the board doesn't reset
 */
void watchdog_feed(void);

/**
 * @brief: Initializes the watchdog
 * @return: 0 on success, errorcode otherwise
 */
int init_watchdog(void);

/**
 * @brief: Disables the watchdog
 */
void watchdog_disable(void);

#endif
