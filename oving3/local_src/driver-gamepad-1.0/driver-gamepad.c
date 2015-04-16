/*
 * This is the Linux kernel module for a gamepad provided in the course TDT4258
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/kdev_t.h>
#include <linux/moduleparam.h>
#include <stdlib.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/siginfo.h>

#include "efm32gg.h"
#include "driver-gamepad.h"i


#define SUCCESS 0
dev_t devNumber;
unsigned int devCount = 1;
struct cdev *buttons_cdev;
struct class *cl;
void __iomem *gpio_mem;
void __iomem *gpio_portc_mem;
static int driverOpen = 0;
/*
 * template_init - function to insert this module into kernel space
 *
 * This is the first of two exported functions to handle inserting this
 * code into a running kernel
 ystem
 * Returns 0 if successfull, otherwise -1
 */

static struct file_operations fops = {
	.owner = THIS_MODULE,
//	.llseek = gamepad_llseek,
	.read = my_read,
    .write = my_write,
//	.mmap = gamepad_map,
	.open = gamepad_open,
	.release = gamepad_release,
};
	
static int __init gamepad_driver_init(void)
{
	printk("Hello World, here is your module speaking\n");

	// Create chardevice and device node
    if( alloc_chrdev_region(&devNumber, 0, devCount, "GPIO_buttons") < 0){
		printk(KERN_ERR "Character device region allocation FAILED, returning.\n");
		return -1;
	}
	cl = class_create(THIS_MODULE, "GPIO_buttons");
	device_create(cl,NULL,devNumber, NULL, "GPIO_buttons");
	

	//Request memory region access for GPIO functions and port C, and check if the driver is in use by other processes
	
	
	if(request_mem_region(GPIO_PA_BASE, 0x20,"GPIO_functions") /*Using PA-adress with offset 0x100 to access GPIO functions*/  == 0)
	{
		printk(KERN_ERR "Port A(GPIO functions) memory request FAILED, returning\n");
		return -1;
	}
	if( request_mem_region(GPIO_PC_BASE, 0x20, "GPIO_port_c") == 0)
	{
		printk(KERN_ERR "Port C memory request FAILED, returning\n");
		return -1;
	}
	

	// Remap to virtual addresses
	gpio_mem = ioremap_nocache(GPIO_PA_BASE, 0x20);  //Using the PA-address to access general GPIO functions
	printk(KERN_DEBUG "gpio_mem_addr: %p\n", gpio_mem);
	if(gpio_mem == 0)
	{
		printk(KERN_ERR "Port A(GPIO) remap failed, returning\n");
		return -1;
	}
	gpio_portc_mem = ioremap_nocache(GPIO_PC_BASE, 0x20);
	printk(KERN_DEBUG "gpio_portc_mem_addr: %p\n", gpio_portc_mem);
	if(gpio_portc_mem == 0)
	{
		printk(KERN_ERR "Port C remap failed, returning\n");
		return -1;
	}

	//Configure interrupts and GPIO, same procedure as ex1 and ex2
    printk(KERN_DEBUG "Config interrupt and GIPO\n");
	iowrite32(0x33333333,   gpio_portc_mem + 0x0c );
    printk(KERN_DEBUG "Bla\n");
    iowrite32(0xff, 	gpio_portc_mem + 0x0c);
    printk(KERN_DEBUG "Bla\n");
    iowrite32(0x22222222,   gpio_mem + 0x100);
    printk(KERN_DEBUG "bla\n");
    iowrite32(0xff, 	gpio_mem + 0x108);
    printk(KERN_DEBUG "blag\n");
    iowrite32(0xff, 	gpio_mem + 0x10c);
    printk(KERN_DEBUG "agag\n");
    iowrite32(0xff, 	gpio_mem + 0x110);

	// Setup GPIO IRQ handler, 17 and 18 are odd and even interrupts
	printk(KERN_DEBUG "Setting up IRQi 17\n");
	if(request_irq(17,(irq_handler_t) interrupt_handler, 0, "GPIO_buttons", NULL) < 0)
	{
		printk(KERN_ERR "IRQ 1 request FAILED, returning \n");
		return -1;
	}
    printk(KERN_DEBUG "Setting up IRQ 18\n");
    if(request_irq(18, (irq_handler_t) interrupt_handler, 0, "GPIO_buttons", NULL) < 0)
	{
		printk(KERN_ERR "IRQ 2 request FAILED, returning \n");
		return -1;
	}

	//might be wise to clear interrupt flags here

    
    
	//Activate driver and register allocations
    printk(KERN_DEBUG "Activating character device\n");
    buttons_cdev = cdev_alloc();
	buttons_cdev->owner = THIS_MODULE;
	buttons_cdev->ops = &fops;
	if(cdev_add(buttons_cdev,devNumber, devCount) < 0)
	{
		printk(KERN_ERR "Char device activation failed, returning\n");
		return -1;
	}


	return 0;
}

/*
 * template_cleanup - function to cleanup this module from kernel space
 *
 * This is the second of two exported functions to handle cleanup this
 * code from a running kernel
 */

static void __exit gamepad_driver_cleanup(void)
{
	 printk("Short life for a small module...\n");

	//Deactivate driver
	printk(KERN_DEBUG "Deactivate driver\n");
	cdev_del(buttons_cdev);
	//Free even and odd interrupts
	free_irq(17,NULL);
	free_irq(18,NULL);

	//Disable GPIO interrupts
	printk(KERN_DEBUG "Disable GPIUO interrupts\n");
	iowrite32(0x0, gpio_mem + 0x110);
	iowrite32(0x0, gpio_mem + 0x108);
	iowrite32(0x0, gpio_mem + 0x10c);

	printk(KERN_DEBUG "Unmap GPIO\n");
	iounmap(gpio_mem);
	iounmap(gpio_portc_mem);

	//Release memory
	printk(KERN_DEBUG "Release memory region\n");
	release_mem_region(GPIO_PA_BASE, 0x20);
	release_mem_region(GPIO_PC_BASE, 0x20);

	//Destroy class and device
	printk(KERN_DEBUG "Destroy class and device\n");
	device_destroy(cl,devNumber);
	class_destroy(cl);

	//Unregister char device region
	printk(KERN_DEBUG "Unregister char device region\n");
	unregister_chrdev_region(devNumber, devCount);
}

/* Functions for using the gamepad from userspace */

static int gamepad_open(struct inode *inode,struct file *file){
    if (driverOpen) {
        return -EBUSY;
    }

    try_module_get(THIS_MODULE); // Prevent module unloading while in use
    driverOpen = 1;
    return SUCCESS; //Configuration handled by the module_init
}

static int gamepad_release(struct inode *inode, struct file *file){
    driverOpen = 0;
    module_put(THIS_MODULE); // Allow module unloading
    return SUCCESS; //Configuration handled by the module_exit
}

// user program reads from the driver
static ssize_t my_read (struct file *filp, char __user *buff, size_t count, loff_t *offp){
    uint32_t data = ioread32(GPIO_PC_DIN);
    char daa[33] = itoa(data);
    copy_to_user(buff, &daa, 33);

    return 1;
}



//user program writes to the driver
static ssize_t my_write (struct file *filp, const char __user *buff, size_t count, loff_t *offp){
    return 0; //Not used as we do not want to write to the LEDs on the gamepad
}

static irq_handler_t interrupt_handler(int irq, void *dev_id, struct pt_regs *regs){
    //TODO: handle interrupts
    iowrite32(0xffff, gpio_mem + 0x11c);//Clear interrupt flags
    printk(KERN_DEBUG "GPIO Interrupt\n");
    return (irq_handler_t) IRQ_HANDLED; 
}



/*
MISSING:


gamepad_read
- reads data from the driver

gamepad_llseek(?)
- ???????

gamepad_mmap
- maps memory to userspace program

interrupt_handler
- handles interrupts


*/



module_init(gamepad_driver_init);
module_exit(gamepad_driver_cleanup);

MODULE_AUTHOR("TDT4258 Group ??");
MODULE_DESCRIPTION("Module for controlling a gamepad provided in the course TDT4258 at NTNU");
MODULE_LICENSE("GPL");

