#include <zephyr/ztest.h>
#include <zephyr/fff.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include "watchdog.h"

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(int, wdt_mock_setup, const struct device *, uint8_t);
FAKE_VALUE_FUNC(int, wdt_mock_install_timeout, const struct device *, const struct wdt_timeout_cfg *);
FAKE_VALUE_FUNC(int, wdt_mock_feed, const struct device *, int);
FAKE_VALUE_FUNC(int, wdt_mock_disable, const struct device *);

static wdt_callback_t stored_callback;
static const struct device *stored_device;

static struct wdt_timeout_cfg stored_cfg;
static bool cfg_captured;

static int wdt_install_timeout_capture_callback(const struct device *dev, const struct wdt_timeout_cfg *cfg)
{
    if (cfg) {
        stored_cfg = *cfg;
        cfg_captured = true;
        stored_callback = cfg->callback;
        stored_device = dev;
    }
    return wdt_mock_install_timeout_fake.return_val;
}

struct dummy_wdt_api {
    int (*setup)(const struct device *dev, uint8_t options);
    int (*disable)(const struct device *dev);
    int (*install_timeout)(const struct device *dev, const struct wdt_timeout_cfg *cfg);
    int (*feed)(const struct device *dev, int channel_id);
};

static struct dummy_wdt_api fake_wdt_api = {
    .setup = wdt_mock_setup,
    .install_timeout = wdt_mock_install_timeout,
    .feed = wdt_mock_feed,
    .disable = wdt_mock_disable,
};

DEVICE_DT_DEFINE(DT_ALIAS(watchdog0), NULL, NULL, NULL, NULL, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
                 &fake_wdt_api);

static void watchdog_tests_before(void *fixture)
{
    RESET_FAKE(wdt_mock_setup);
    RESET_FAKE(wdt_mock_install_timeout);
    RESET_FAKE(wdt_mock_feed);
    RESET_FAKE(wdt_mock_disable);
    FFF_RESET_HISTORY();

    stored_callback = NULL;
    stored_device = NULL;
    cfg_captured = false;

    wdt_mock_setup_fake.return_val = 0;
    wdt_mock_install_timeout_fake.return_val = 5;
    wdt_mock_feed_fake.return_val = 0;
    wdt_mock_disable_fake.return_val = 0;

    wdt_mock_install_timeout_fake.custom_fake = wdt_install_timeout_capture_callback;

    const struct device *dev = DEVICE_DT_GET(DT_ALIAS(watchdog0));
    ((struct device *)dev)->state->initialized = true;
}

ZTEST(watchdog_tests, test_init_watchdog_success)
{
    int ret = init_watchdog();

    zassert_equal(ret, 0, NULL);
    zassert_equal(wdt_mock_install_timeout_fake.call_count, 1, NULL);
    zassert_equal(wdt_mock_setup_fake.call_count, 1, NULL);
    zassert_equal(wdt_mock_setup_fake.arg1_val, WDT_OPT_PAUSE_HALTED_BY_DBG, NULL);
}

ZTEST(watchdog_tests, test_init_watchdog_device_not_ready)
{
    const struct device *dev = DEVICE_DT_GET(DT_ALIAS(watchdog0));
    ((struct device *)dev)->state->initialized = false;

    int ret = init_watchdog();

    zassert_equal(ret, -ENODEV, NULL);
    zassert_equal(wdt_mock_install_timeout_fake.call_count, 0, NULL);
    zassert_equal(wdt_mock_setup_fake.call_count, 0, NULL);
}

ZTEST(watchdog_tests, test_init_watchdog_install_timeout_fails)
{
    wdt_mock_install_timeout_fake.return_val = -EINVAL;

    int ret = init_watchdog();

    zassert_equal(ret, -EINVAL, NULL);
    zassert_equal(wdt_mock_install_timeout_fake.call_count, 1, NULL);
    zassert_equal(wdt_mock_setup_fake.call_count, 0, NULL);
}

ZTEST(watchdog_tests, test_init_watchdog_setup_fails)
{
    wdt_mock_setup_fake.return_val = -EIO;

    int ret = init_watchdog();

    zassert_equal(ret, -EIO, NULL);
    zassert_equal(wdt_mock_install_timeout_fake.call_count, 1, NULL);
    zassert_equal(wdt_mock_setup_fake.call_count, 1, NULL);
}

ZTEST(watchdog_tests, test_init_watchdog_timeout_config)
{
    init_watchdog();

    zassert_true(cfg_captured, NULL);
    zassert_equal(stored_cfg.window.min, 0, NULL);
    zassert_equal(stored_cfg.window.max, WDT_TIMEOUT_MS, NULL);
    zassert_not_null(stored_cfg.callback, NULL);
    zassert_equal(stored_cfg.flags, WDT_FLAG_RESET_SOC, NULL);
}

ZTEST(watchdog_tests, test_init_watchdog_callback_registered)
{
    init_watchdog();

    zassert_not_null(stored_callback, NULL);
}

ZTEST(watchdog_tests, test_watchdog_feed_success)
{
    wdt_mock_install_timeout_fake.return_val = 7;
    init_watchdog();

    RESET_FAKE(wdt_mock_feed);

    watchdog_feed();

    zassert_equal(wdt_mock_feed_fake.call_count, 1, NULL);
    zassert_equal(wdt_mock_feed_fake.arg1_val, 7, NULL);
}

ZTEST(watchdog_tests, test_watchdog_feed_multiple_calls)
{
    wdt_mock_install_timeout_fake.return_val = 3;
    init_watchdog();

    RESET_FAKE(wdt_mock_feed);

    watchdog_feed();
    watchdog_feed();
    watchdog_feed();

    zassert_equal(wdt_mock_feed_fake.call_count, 3, NULL);
}

ZTEST(watchdog_tests, test_watchdog_feed_uses_correct_device)
{
    init_watchdog();

    RESET_FAKE(wdt_mock_feed);

    watchdog_feed();

    const struct device *expected = DEVICE_DT_GET(DT_ALIAS(watchdog0));

    zassert_equal(wdt_mock_feed_fake.arg0_val, expected, NULL);
}

ZTEST(watchdog_tests, test_watchdog_callback_invocation)
{
    init_watchdog();

    zassert_not_null(stored_callback, NULL);

    if (stored_callback && stored_device) {
        stored_callback(stored_device, 5);
        zassert_true(true, NULL);
    }
}

ZTEST(watchdog_tests, test_watchdog_disable_success)
{
    init_watchdog();

    watchdog_disable();

    zassert_equal(wdt_mock_disable_fake.call_count, 1, NULL);
    const struct device *expected = DEVICE_DT_GET(DT_ALIAS(watchdog0));
    zassert_equal(wdt_mock_disable_fake.arg0_val, expected, NULL);
}

ZTEST_SUITE(watchdog_tests, NULL, NULL, watchdog_tests_before, NULL, NULL);
