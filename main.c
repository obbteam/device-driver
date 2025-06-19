#include <linux/module.h>
#include <linux/platform_device.h>
#include "device_file.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Seven-segment wrapper that registers platform driver");
MODULE_AUTHOR("obbteam");

module_platform_driver(sevenseg_driver);