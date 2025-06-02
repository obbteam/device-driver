#include <linux/init.h>
#include <linux/module.h>
#include "device_file.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple linux driver");
MODULE_AUTHOR("obbteam");

static int my_init(void)
{
    int result = 0;
    printk(KERN_NOTICE "Simple-driver: Initialization started\n");

    result = register_device();
    return 0;
}

static void my_exit(void)
{
    printk(KERN_NOTICE "Simple-driver: Exiting\n");
    unregister_device();
}

module_init(my_init);
module_exit(my_exit);