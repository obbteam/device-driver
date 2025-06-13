#include "device_file.h"
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/miscdevice.h>
#include <linux/gpio/consumer.h>

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

/*============ DYNAMICALLY REGISTERING THE DEVICE ============ */
static struct miscdevice simple_misc =
    {
        .minor = MISC_DYNAMIC_MINOR,
        .name = device_name,
        .fops = &simple_driver_fops,
};

int register_device(void)
{
    int result = 0;
    printk(KERN_NOTICE "%s: Register_device() is called\n", device_name);
    // result = register_chrdev(0, device_name, &simple_driver_fops);
    result = misc_register(&simple_misc);

    if (result)
    {
        printk(KERN_WARNING "%s: Misc_register failed (%d)\n", device_name, result);
        return result;
    }

    segs = devm_gpiod_get_array(simple_misc.this_device, "seg", GPIOD_OUT_LOW);

    if (IS_ERR(segs))
    {
        printk(KERN_ERR "%s: failed to get seg gpios: %ld\n", device_name, segs);
        int ret = PTR_ERR(segs);
        misc_deregister(&simple_misc);
        return ret;
    }

    printk(KERN_NOTICE "%s: Loaded, /dev/%s ready\n", device_name, device_name);

    return 0;
}

void unregister_device(void)
{
    printk(KERN_NOTICE "%s: Unregister_device() is called\n", device_name);
    misc_deregister(&simple_misc);
    printk(KERN_NOTICE "%s: Unloaded\n", device_name);
}
