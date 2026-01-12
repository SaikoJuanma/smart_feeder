#include <zephyr/kernel.h>

int main(void)
{
    printk("Hello world\n");
    k_sleep(K_MSEC(20));

    return 0;
}
