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

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/siginfo.h>

#include "efm32gg.h"
#include "driver-gamepad.h"

dev_t devNumber;
unsigned int devCount = 1;
struct cdev *buttons_cdev;
struct class *cl;
void __iomem *gpio_mem;
void __iomem *gpio_portc_mem;

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
	.read = gamepad_read,
//	.mmap = gamepad_map,
	.open = gamepad_open,
	.release = gamepad_release,
};
	
static int __init gamepad_driver_init(void)
{
	printk("Hello World, here is your module speaking\n");

	// Create chardevice and device node
	int error;
    error = alloc_chardev_region(&devNumber, 0, devCount, "GPIO_buttons");
	if(error < 0) {
		printk(KERN_ERR "Character device region allocation FAILED, returning.\n");
		return -1;
	}
	cl = class_create(THIS_MODULE, "GPIO_buttons");
	device_create(cl,NULL,devNumber, NULL, "GPIO_buttons");
	

	//Request memory region access for GPIO functions and port C, and check if the driver is in use by other processes
	
	
	struct resource *GPIO_resource = request_mem_region(GPIO_PA_BASE + 0x100, 0x20,"GPIO_functions"); //Using PA-adress with offset 0x100 to access GPIO functions
	if(GPIO_resource == 0)
	{
		printk(KERN_ERR "Port A(GPIO functions) memory request FAILED, returning\n");
		return -1;
	}
	struct resource *portC_resource;
    *portC_resource= request_mem_region(GPIO_PC_BASE, 0x24, "GPIO_port_c");
	if(portC_resource == 0)
	{
		printk(KERN_ERR "Port C memory request FAILED, returning\n");
		return -1;
	}
	

	// Remap to virtual addresses
	gpio_mem = ioremap_nocache(GPIO_PA_BASE+0x100, 0x20);  //Using the PA-address to access general GPIO functions with offset 0x100
	printk(KERN_DEBUG "gpio_mem_addr: %p\n", gpio_mem);
	if(gpio_mem == 0)
	{
		printk(KERN_ERR "Port A(GPIO) remap failed, returning\n");
		return -1;
	}
	gpio_portc_mem = ioremap_nocache(GPIO_PC_BASE, 0x24);
	printk(KERN_DEBUG "gpio_portc_mem_addr: %p\n", gpio_portc_mem);
	if(gpio_portc_mem == 0)
	{
		printk(KERN_ERR "Port C remap failed, returning\n");
		return -1;
	}

	//Configure interrupts and GPIO, same procedure as ex1 and ex2

	iowrite32(0x33333333,   gpio_portc_mem + *GPIO_PC_MODEL);
	iowrite32(0xff, 	gpio_portc_mem + *GPIO_PC_DOUT);
	iowrite32(0x22222222,   gpio_mem + *GPIO_EXTIPSELL);
	iowrite32(0xff, 	gpio_mem + *GPIO_EXTIRISE);
	iowrite32(0xff, 	gpio_mem + *GPIO_EXTIFALL);
	iowrite32(0xff, 	gpio_mem + *GPIO_IEN);

	// Setup GPIO IRQ handler, 17 and 18 are odd and even interrupts
	
	error = request_irq(17, interrupt_handler, 0, "GPIO_buttons",NULL);
	if(error < 0)
	{
		printk(KERN_ERR "IRQ 1 request FAILED, returning \n");
		return -1;
	}
	error = request_irq(18, interrupt_handler, 0, "GPIO_buttons", NULL);
	if(error < 0)
	{
		printk(KERN_ERR "IRQ 2 request FAILED, returning \n");
		return -1;
	}

	//might be wise to clear interrupt flags here



	//Activate driver and register allocations
	buttons_cdev = cdev_alloc();
	buttons_cdev->owner = THIS_MODULE;
	buttons_cdev->ops = &fops;
	error = cdev_add(buttons_cdev,devNumber, devCount);
	if(error == 0)
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
	iowrite32(0x0, gpio_mem + *GPIO_IEN);
	iowrite32(0x0, gpio_mem + *GPIO_EXTIRISE);
	iowrite32(0x0, gpio_mem + *GPIO_EXTIFALL);

	printk(KERN_DEBUG "Unmap GPIO\n");
	iounmap(gpio_mem);
	iounmap(gpio_portc_mem);

	//Release memory
	printk(KERN_DEBUG "Release memory region\n");
	release_mem_region(GPIO_PA_BASE + 0x100, 0x20);
	release_mem_region(GPIO_PC_BASE, 0x24);

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
	return 0; //Configuration handled by the module_init
}

static int gamepad_release(struct inode *inode, struct file *file){
	return 0; //Configuration handled by the module_exit
}

// user program reads from the driver
static ssize_t my_read (struct file *filp, char __user *buff, size_t count, loff_t *offp){
    uint32_t data = ioread32(GPIO_PC_DIN);
    copy_to_user(buff, &data, 1);

    return 1;
}



//user program writes to the driver
static ssize_t my_write (struct file *filp, const char __user *buff, size_t count, loff_t *offp){
    return 0; //Not used as we do not want to write to the LEDs on the gamepad
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

MODULE_DESCRIPTION("Module for controlling a gamepad provided in the course TDT4258 at NTNU");
MODULE_LICENSE("GPL");

