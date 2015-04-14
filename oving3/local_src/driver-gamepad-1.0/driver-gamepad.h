#ifndef DRIVER_GAMEPAD_H 
#define DRIVER_GAMEPAD_H 


static int gamepad_open(struct inode *inode, struct file *file);
static int gamepad_release(struct inode *inode, struct file *file);
static ssize_t my_read(struct file *filp, char __user *buff, size_t count, loff_t *offp);
static ssize_t my_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);




#endif
