#include "device_file.h"
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/miscdevice.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>

/* ============ DRIVER INIT VARS ============*/

static const char device_name[] = "akhmadkhonov-driver";

/* ============ 7 segment MACROS ============*/

static struct gpio_descs *segs; /* segs->ndescs == SEGMENT_COUNT */
#define SEG_COUNT 8

void clear_seven_segment()
{
    printk(KERN_NOTICE "%s: Device file is clearing 7 segment\n", device_name);

    for (int i = 0; i < SEG_COUNT; ++i)
    {
        gpiod_set_value_cansleep(segs->desc[i], 0);
    }
}

void write_num(int num)
{
    clear_seven_segment();

    printk(KERN_NOTICE "%s: Device file is writing to seven segment num: %d\n", device_name, num);

    switch (num)
    {
    case 0:
        gpiod_set_value_cansleep(segs->desc[0], 1);
        gpiod_set_value_cansleep(segs->desc[1], 1);
        gpiod_set_value_cansleep(segs->desc[2], 1);
        gpiod_set_value_cansleep(segs->desc[3], 1);
        gpiod_set_value_cansleep(segs->desc[4], 1);
        gpiod_set_value_cansleep(segs->desc[5], 1);
        break;
    case 1:
        gpiod_set_value_cansleep(segs->desc[1], 1);
        gpiod_set_value_cansleep(segs->desc[2], 1);
        break;
    case 2:
        gpiod_set_value_cansleep(segs->desc[0], 1);
        gpiod_set_value_cansleep(segs->desc[1], 1);
        gpiod_set_value_cansleep(segs->desc[6], 1);
        gpiod_set_value_cansleep(segs->desc[4], 1);
        gpiod_set_value_cansleep(segs->desc[3], 1);
        break;
    case 3:
        gpiod_set_value_cansleep(segs->desc[0], 1);
        gpiod_set_value_cansleep(segs->desc[1], 1);
        gpiod_set_value_cansleep(segs->desc[2], 1);
        gpiod_set_value_cansleep(segs->desc[3], 1);
        gpiod_set_value_cansleep(segs->desc[6], 1);
        break;
    case 4:
        gpiod_set_value_cansleep(segs->desc[1], 1);
        gpiod_set_value_cansleep(segs->desc[2], 1);
        gpiod_set_value_cansleep(segs->desc[5], 1);
        gpiod_set_value_cansleep(segs->desc[6], 1);
        break;
    case 5:
        gpiod_set_value_cansleep(segs->desc[0], 1);
        gpiod_set_value_cansleep(segs->desc[2], 1);
        gpiod_set_value_cansleep(segs->desc[3], 1);
        gpiod_set_value_cansleep(segs->desc[5], 1);
        gpiod_set_value_cansleep(segs->desc[6], 1);
        break;
    case 6:
        gpiod_set_value_cansleep(segs->desc[0], 1);
        gpiod_set_value_cansleep(segs->desc[2], 1);
        gpiod_set_value_cansleep(segs->desc[3], 1);
        gpiod_set_value_cansleep(segs->desc[4], 1);
        gpiod_set_value_cansleep(segs->desc[5], 1);
        gpiod_set_value_cansleep(segs->desc[6], 1);
        break;
    case 7:
        gpiod_set_value_cansleep(segs->desc[0], 1);
        gpiod_set_value_cansleep(segs->desc[1], 1);
        gpiod_set_value_cansleep(segs->desc[2], 1);
        break;
    case 8:
        for (int i = 0; i < 7; i++)
            gpiod_set_value_cansleep(segs->desc[i], 1);
        break;
    case 9:
        gpiod_set_value_cansleep(segs->desc[0], 1);
        gpiod_set_value_cansleep(segs->desc[1], 1);
        gpiod_set_value_cansleep(segs->desc[2], 1);
        gpiod_set_value_cansleep(segs->desc[3], 1);
        gpiod_set_value_cansleep(segs->desc[5], 1);
        gpiod_set_value_cansleep(segs->desc[6], 1);
        break;
    default: /* dot only */
        gpiod_set_value_cansleep(segs->desc[7], 1);
    }
}

/* ============ DYNAMIC READ/WRITE IMPLEMENTATION ============ */
#define BUF_MAX 255

static DEFINE_MUTEX(buf_lock); // Mutex to protect buffer for a write command
static char buffer[BUF_MAX];
static size_t buf_len;

static ssize_t device_file_read_dynamic(struct file *file_ptr,
                                        char __user *user_buffer,
                                        size_t count,
                                        loff_t *position)
{
    printk(KERN_NOTICE "%s: Device file is reading dynamically at offset = %i, read bytes count = %u\n",
           device_name,
           (int)*position,
           (unsigned int)count);

    if (*position >= buf_len)
        return 0;
    if (*position + count > buf_len)
        count = buf_len - *position;
    if (copy_to_user(user_buffer, buffer + *position, count) != 0)
        return -EFAULT;
    *position += count;
    return count;
}

static ssize_t device_file_write(struct file *file_ptr,
                                 const char __user *user_buffer,
                                 size_t count,
                                 loff_t *position)
{
    ssize_t n = min(count, BUF_MAX);

    mutex_lock(&buf_lock);

    buf_len = 0;

    printk(KERN_NOTICE "%s: Device file recieved %i, written bytes count = %u\n",
           device_name,
           (unsigned int)count,
           (unsigned int)n);

    if (copy_from_user(buffer, user_buffer, n) != 0)
    {
        mutex_unlock(&buf_lock);
        return -EFAULT;
    }

    // we simply write the first digit from the stream
    write_num((int)buffer[0] - 48);

    *position = n;
    buf_len = n;
    mutex_unlock(&buf_lock);

    return n;
}

static int device_open(struct inode *inode, struct file *filp)
{
    filp->f_pos = 0; /* start every session at offset 0 */
    return 0;
}

static struct file_operations simple_driver_fops = {
    .open = device_open,
    .owner = THIS_MODULE,
    .read = device_file_read_dynamic,
    .write = device_file_write,
};

static struct miscdevice sevenseg_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = device_name,
    .fops = &simple_driver_fops,
};

/*============ PLATFORM DRIVER GLUE ============ */

static int sevenseg_probe(struct platform_device *pdev)
{
    int ret;

    /* 1. Obtain the “seg” GPIO array defined in DT */
    segs = devm_gpiod_get_array(&pdev->dev, "seg", GPIOD_OUT_LOW);
    if (IS_ERR(segs))
        return PTR_ERR(segs);

    /* 2. Hook misc device to this parent so the DT link shows in sysfs */
    sevenseg_misc.parent = &pdev->dev;

    /* 3. Register /dev/akhmadkhonov-driver */
    ret = misc_register(&sevenseg_misc);
    if (ret)
        return ret;

    dev_info(&pdev->dev,
             "seven-segment ready as /dev/%s (segments=%d)\n",
             sevenseg_misc.name, segs->ndescs);
    return 0; /* probe success */
}

static void sevenseg_remove(struct platform_device *pdev)
{
    misc_deregister(&sevenseg_misc);
}

static const struct of_device_id sevenseg_of_match[] = {
    {.compatible = "akhmadkhonov,sevenseg"},
    {/* sentinel */}};
MODULE_DEVICE_TABLE(of, sevenseg_of_match);

struct platform_driver sevenseg_driver = {
    .driver = {
        .name = "akhmadkhonov-sevenseg",
        .of_match_table = sevenseg_of_match,
    },
    .probe = sevenseg_probe,
    .remove = sevenseg_remove,
};