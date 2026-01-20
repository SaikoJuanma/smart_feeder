#include <zephyr/fs/nvs.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/logging/log.h>
#include "configuration.h"

LOG_MODULE_REGISTER(configuration, LOG_LEVEL_INF);

struct config cfg;
struct nvs_fs fs;

int init_nvs(void)
{
    struct flash_pages_info info;
    int ret;

    fs.flash_device = FIXED_PARTITION_DEVICE(storage_partition);
    if (!device_is_ready(fs.flash_device)) {
        LOG_ERR("Flash device not ready\n");
        return -ENODEV;
    }

    fs.offset = FIXED_PARTITION_OFFSET(storage_partition);

    ret = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
    if (ret) {
        LOG_ERR("Unable to get page info: %d\n", ret);
        return ret;
    }

    fs.sector_size = info.size;
    fs.sector_count = 3;
    ret = nvs_mount(&fs);
    if (ret) {
        LOG_ERR("NVS mount failed: %d\n", ret);
        return ret;
    }

    LOG_INF("NVS initialized\n");
    return 0;
}

int save_config(void)
{
    int ret;

    ret = nvs_write(&fs, CONFIG_ID, &cfg, sizeof(cfg));
    if (ret < 0) {
        LOG_ERR("Failed to write config: %d\n", ret);
        return ret;
    }

    LOG_INF("Saved %d bytes\n", ret);
    return 0;
}

int load_config(void)
{
    int ret;

    ret = nvs_read(&fs, CONFIG_ID, &cfg, sizeof(cfg));
    if (ret < 0) {
        LOG_ERR("Failed to read config: %d\n", ret);
        return ret;
    }

    LOG_INF("Loaded %d bytes\n", ret);
    return 0;
}

void set_dflt_cfg(void)
{
    cfg.random_value = 0;
}
