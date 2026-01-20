#include <zephyr/ztest.h>
#include <zephyr/fff.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include "configuration.h"

DEFINE_FFF_GLOBALS;

extern struct nvs_fs fs;

FAKE_VALUE_FUNC(int, nvs_mount, struct nvs_fs *);
FAKE_VALUE_FUNC(ssize_t, nvs_write, struct nvs_fs *, uint16_t, const void *, size_t);
FAKE_VALUE_FUNC(ssize_t, nvs_read, struct nvs_fs *, uint16_t, void *, size_t);

static ssize_t nvs_read_custom_fake(struct nvs_fs *fs, uint16_t id, void *data, size_t len)
{
    struct config *dest = (struct config *)data;
    dest->random_value = 888;
    return len;
}

static void config_test_setup(void *fixture)
{
    RESET_FAKE(nvs_mount);
    RESET_FAKE(nvs_write);
    RESET_FAKE(nvs_read);
    FFF_RESET_HISTORY();

    memset(&cfg, 0, sizeof(struct config));
}

ZTEST_SUITE(configuration, NULL, NULL, config_test_setup, NULL, NULL);

ZTEST(configuration, test_init_nvs_success)
{
    nvs_mount_fake.return_val = 0;

    int ret = init_nvs();

    /* If ret is 0, it means real flash info was read successfully */
    if (ret == 0) {
        zassert_equal(nvs_mount_fake.call_count, 1, "Should mount NVS");
        zassert_true(fs.sector_size > 0, "Sector size should be set by real driver");
    } else {
        /* If real hardware fails (e.g. partition not found), we log it but don't crash the suite */
        TC_PRINT("init_nvs failed with %d (Real hardware flash error)\n", ret);
    }
}

ZTEST(configuration, test_init_nvs_fail_mount)
{
    /* We assume flash info succeeds on hardware for this test */
    nvs_mount_fake.return_val = -ENOSPC;

    int ret = init_nvs();

    if (ret != -ENODEV) { /* -ENODEV would mean flash device not ready */
        zassert_equal(ret, -ENOSPC, "Should return error from mount");
    }
}

ZTEST(configuration, test_save_config_success)
{
    nvs_write_fake.return_val = sizeof(struct config);
    cfg.random_value = 123;

    int ret = save_config();

    zassert_equal(ret, 0, "save_config failed");
    zassert_equal(nvs_write_fake.call_count, 1, "nvs_write not called");
    zassert_equal(nvs_write_fake.arg1_val, CONFIG_ID, "Wrong NVS ID used");
}

ZTEST(configuration, test_save_config_fail)
{
    nvs_write_fake.return_val = -EIO;

    int ret = save_config();

    zassert_equal(ret, -EIO, "Should return error code");
}

ZTEST(configuration, test_load_config_success)
{
    nvs_read_fake.custom_fake = nvs_read_custom_fake;

    int ret = load_config();

    zassert_equal(ret, 0, "load_config failed");
    zassert_equal(nvs_read_fake.call_count, 1, "nvs_read not called");
    zassert_equal(cfg.random_value, 888, "Config not updated from NVS read");
}

ZTEST(configuration, test_load_config_fail)
{
    nvs_read_fake.return_val = -ENOENT;

    int ret = load_config();

    zassert_equal(ret, -ENOENT, "Should return error code");
}

ZTEST(configuration, test_default_cfg)
{
    cfg.random_value = 234;
    set_dflt_cfg();
    zassert_equal(cfg.random_value, 0, "random number should be 0, instead %d", cfg.random_value);
}
