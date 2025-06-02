#include "device_file.h"
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

/* ============ DRIVER INIT VARS ============*/

static int device_file_major_number = 0;
static const char device_name[] = "Akhmadkhonov-driver";

/* ============ SIMPLE READ IMPLEMENTATION ============ */

/*
static const char g_s_Hello_world_string[] = "Hi Mr.Felix from obbteam's kernel mode!\n";
static const ssize_t g_s_Hello_world_size = sizeof(g_s_Hello_world_string);

static ssize_t device_file_read_simple(struct file *file_ptr,
                                       char __user *user_buffer,
                                       size_t count,
                                       loff_t *position)
{
    printk(KERN_NOTICE "Akhmadkhonov-driver: Device file is read at offset = %i, read bytes count = %un",
           (int)*position,
           (unsigned int)count);

    if (*position >= g_s_Hello_world_size)
        return 0;
    if (*position + count > g_s_Hello_world_size)
        count = g_s_Hello_world_size - *position;
    if (copy_to_user(user_buffer, g_s_Hello_world_string + *position, count) != 0)
        return -EFAULT;
    *position += count;
    return count;
}
*/

/* ============ DYNAMIC READ/WRITE IMPLEMENTATION ============ */
#define BUF_MAX 50

static DEFINE_MUTEX(buf_lock); // Mutex to protect buffer for a write command
static char buffer[BUF_MAX];
static size_t buf_len;

static ssize_t device_file_read_dynamic(struct file *file_ptr,
                                        char __user *user_buffer,
                                        size_t count,
                                        loff_t *position)
{
    printk(KERN_NOTICE "Akhmadkhonov-driver: Device file is reading dynamically at offset = %i, read bytes count = %un",
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

    printk(KERN_NOTICE "Akhmadkhonov-driver: Device file recieved %i, written bytes count = %un",
           (unsigned int)count,
           (unsigned int)n);

    if (copy_from_user(buffer, user_buffer, n) != 0)
    {
        mutex_unlock(&buf_lock);
        return -EFAULT;
    }

    position += n;
    buf_len = n;
    mutex_unlock(&buf_lock);

    return n;
}

static struct file_operations simple_driver_fops = {
    .owner = THIS_MODULE,
    .read = device_file_read_dynamic,
    .write = device_file_write,
};

int register_device(void)
{
    int result = 0;
    printk(KERN_NOTICE "Akhmadkhonov-driver: register_device() is called.n");
    result = register_chrdev(0, device_name, &simple_driver_fops);
    if (result < 0)
    {
        printk(KERN_WARNING "Akhmadkhonov-driver: can't register character device with error code = %in", result);
        return result;
    }

    device_file_major_number = result;
    printk(KERN_NOTICE "Akhmadkhonov-driver: registered character device with major number = %i and minor numbers  0...255n", device_file_major_number);
    return 0;
}

void unregister_device(void)
{
    printk(KERN_NOTICE "Akhmadkhonov-driver: unregister_device() is called.n");
    if (device_file_major_number != 0)
    {
        unregister_chrdev(device_file_major_number, device_name);
    }
}
