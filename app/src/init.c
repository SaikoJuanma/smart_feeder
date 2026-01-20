#include <zephyr/logging/log.h>
#include "init.h"
#include "configuration.h"

LOG_MODULE_REGISTER(init, LOG_LEVEL_INF);

// TODO: definition and unit test left
int processes_init(void)
{
    int ret;

    LOG_INF("Starting initialization...");

    ret = init_nvs();
    if (ret < 0) {
        LOG_ERR("NVS init failed: %d", ret);
        return ret;
    }

    ret = load_config();
    if (ret < 0) {
        LOG_INF("No saved config, using defaults");
        set_dflt_cfg();
    }

    LOG_INF("Initialization complete");
    return 0;
}
