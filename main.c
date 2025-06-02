#include <linux/init.h>
#include <linux/module.h>
#include "device_file.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Read/write linux driver");
MODULE_AUTHOR("obbteam");

static int my_init(void)
{
    int result = 0;
    printk(KERN_NOTICE "Akhmadkhonov-driver: Initialization started\n");

    result = register_device();
    return 0;
}

static void my_exit(void)
{
    unregister_device();
    printk(KERN_NOTICE "Akhmadkhonov-driver: Exiting\n");
}

module_init(my_init);
module_exit(my_exit);