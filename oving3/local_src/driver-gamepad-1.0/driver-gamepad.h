#ifndef DRIVER_GAMEPAD_H 
#define DRIVER_GAMEPAD_H 


static int gp_open(struct inode *inode, struct file *file);
static int gp_release(struct inode *inode, struct file *file);
static ssize_t gp_read(struct file *filp, char __user *buff, size_t count, loff_t *offp);
static ssize_t gp_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);
static irq_handler_t interrupt_handler(int irq, void *dev_id, struct pt_regs *regs);
static void button_map(void);
static int gp_fasync(int fd, struct file *filp, int mode);



#endif
