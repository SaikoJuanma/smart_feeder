#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "init.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    processes_init();

    while (1) {
        k_sleep(K_MSEC(20));
    }

    return 0;
}
