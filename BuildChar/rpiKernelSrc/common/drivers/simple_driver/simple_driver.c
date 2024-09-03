#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "simple_driver"
#define BUFFER_SIZE 1024

static int major;
static char device_buffer[BUFFER_SIZE];
static int device_open_count = 0;

static int device_open(struct inode *inode, struct file *file) {
    device_open_count++;
    pr_info("simple_char: Device opened\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    device_open_count--;
    pr_info("simple_char: Device closed\n");
    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t len, loff_t *offset) {
    int bytes_read = len < BUFFER_SIZE ? len : BUFFER_SIZE;
    if (copy_to_user(buffer, device_buffer, bytes_read)) {
        pr_err("simple_char: Failed to send data to user\n");
        return -EFAULT;
    }
    pr_info("simple_char: Read %d bytes from device\n", bytes_read);
    return bytes_read;
}

static ssize_t device_write(struct file *filp, const char *buffer, size_t len, loff_t *offset) {
    int bytes_to_write = len < BUFFER_SIZE ? len : BUFFER_SIZE;
    if (copy_from_user(device_buffer, buffer, bytes_to_write)) {
        pr_err("simple_char: Failed to receive data from user\n");
        return -EFAULT;
    }
    pr_info("simple_char: Written %d bytes to device\n", bytes_to_write);
    return bytes_to_write;
}

static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
};

static int __init simple_char_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        pr_err("simple_char: Failed to register device\n");
        return major;
    }
    pr_info("simple_char: Device registered with major number %d\n", major);
    return 0;
}

static void __exit simple_char_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("simple_char: Device unregistered\n");
}

module_init(simple_char_init);
module_exit(simple_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sid");
MODULE_DESCRIPTION("A Simple Character Device Driver");
