/**
 * @file: watchdog.c
 * @brief: watchdog handling.
 *
 * All the functions that focus on controlling or maninpulating the watchdog
 */
#include <zephyr/drivers/watchdog.h>
#include <zephyr/logging/log.h>
#include "watchdog.h"

LOG_MODULE_REGISTER(watchdog, LOG_LEVEL_INF);

const struct device *wdt;
int wdt_channel_id;

/* Local prototypes */
static void wdt_callback(const struct device *dev, int channel_id);

int init_watchdog(void)
{
    int ret;
    // TODO: add a functions that prints reset stuf before reseting
    struct wdt_timeout_cfg wdt_config = {
        .window.min = 0, .window.max = WDT_TIMEOUT_MS, .callback = wdt_callback, .flags = WDT_FLAG_RESET_SOC};

    wdt = DEVICE_DT_GET(DT_ALIAS(watchdog0));
    if (!device_is_ready(wdt)) {
        LOG_ERR("Watchdog device is not ready");
        return -ENODEV;
    }

    wdt_channel_id = wdt_install_timeout(wdt, &wdt_config);
    if (wdt_channel_id < 0) {
        LOG_ERR("Watchdog intall error: %d", wdt_channel_id);
        return wdt_channel_id;
    }

    ret = wdt_setup(wdt, WDT_OPT_PAUSE_HALTED_BY_DBG);
    if (ret < 0) {
        LOG_ERR("Watchdog setup error: %d", ret);
        return ret;
    }

    LOG_INF("Watchdog initialized with %dms timeout", WDT_TIMEOUT_MS);
    return 0;
}

void watchdog_feed(void)
{
    wdt_feed(wdt, wdt_channel_id);
}

/**
 * @brief: function that is called when the watchdog is activated.
 */
static void wdt_callback(const struct device *dev, int channel_id)
{
    LOG_ERR("!!! WATCHDOG TIMEOUT !!! System will reset NOW");
    LOG_ERR("Channel: %d", channel_id);
}

void watchdog_disable(void)
{
    wdt_disable(wdt);
}
