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
#include "driver-gamepad.h"i


#define SUCCESS 0
dev_t devNumber;
unsigned int devCount = 1;
struct cdev *buttons_cdev;
struct class *cl;
void __iomem *gpio_porta_mem;
void __iomem *gpio_portc_mem;
void __iomem *gpio_int_mem;
static int driverOpen = 0;

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.llseek = noop_llseek,
	.read = my_read,
    .write = my_write,
//	.mmap = gamepad_map,
	.open = gamepad_open,
	.release = gamepad_release,
};
	

// Called on module load
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
	if(request_mem_region(GPIO_PA_BASE, 0x24,"GPIO_port_a") == 0)
	{
		printk(KERN_ERR "Port A memory request failed\n");
		return -1;
	}
	if( request_mem_region(GPIO_PC_BASE, 0x24, "GPIO_port_c") == 0)
	{
		printk(KERN_ERR "Port C memory request failed\n");
		return -1;
	}
	if (request_mem_region(GPIO_INT_BASE, 0x10, "GPIO_int") == 0) 
    {
        printk(KERN_ERR "GPIO Interrupt memory request failed\n");
        return -1;
    }

	// Remap to virtual addresses
	gpio_porta_mem = ioremap_nocache(GPIO_PA_BASE, 0x24);
	printk(KERN_DEBUG "GAMEPAD:gpio_porta_mem_addr: %p\n", gpio_porta_mem);
	if(gpio_porta_mem == 0)
	{
		printk(KERN_ERR "Port A remap failed\n");
		return -1;
	}
	gpio_portc_mem = ioremap_nocache(GPIO_PC_BASE, 0x24);
	printk(KERN_DEBUG "GAMEPAD:gpio_portc_mem_addr: %p\n", gpio_portc_mem);
	if(gpio_portc_mem == 0)
	{
		printk(KERN_ERR "Port C remap failed\n");
		return -1;
	}
    gpio_int_mem = ioremap_nocache(GPIO_INT_BASE, 0x10);
    printk(KERN_DEBUG "GAMEPAD:gpio_int_mem_addr: %p\n", gpio_int_mem);
    if(gpio_int_mem == 0) {
        printk(KERN_ERR "GPIO Interrupt remap failed");
        return -1;
    }


    printk(KERN_DEBUG "GAMEPAD:Config interrupt and GIPO\n");
	// Set pint 0-7 as input
    iowrite32(0x33333333,   gpio_portc_mem + MODEL_OFFSET );
    // Set internal pullup
    iowrite32(0xff, 	gpio_portc_mem + DOUT_OFFSET);
    // Enable interrupt on port C
    iowrite32(0x22222222,   gpio_int_mem + EXTIPSELL_OFFSET);
    // Enable interrupt on rising edge
    iowrite32(0xff, gpio_int_mem + EXTIRISE_OFFSET);
    // Enable interrupt on falling edge
    iowrite32(0xff, gpio_int_mem + EXTIFALL_OFFSET);
    // Enable interrupt
    iowrite32(0xff, gpio_int_mem + IEN_OFFSET);

	// Setup GPIO IRQ handler, 17 and 18 are odd and even interrupts
	printk(KERN_DEBUG "GAMEPAD:Setting up IRQi 17\n");
	if(request_irq(17,(irq_handler_t) interrupt_handler, 0, "GPIO_buttons", NULL) < 0)
	{
		printk(KERN_ERR "IRQ 1 request FAILED, returning \n");
		return -1;
	}
    printk(KERN_DEBUG "GAMEPAD:Setting up IRQ 18\n");
    if(request_irq(18, (irq_handler_t) interrupt_handler, 0, "GPIO_buttons", NULL) < 0)
	{
		printk(KERN_ERR "IRQ 2 request FAILED, returning \n");
		return -1;
	}

	//might be wise to clear interrupt flags here

    
    
	//Activate driver and register allocations
    printk(KERN_DEBUG "GAMEPAD:Activating character device\n");
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
	//Deactivate driver
	printk(KERN_DEBUG "GAMEPAD:GAMEPAD: Deactivate driver\n");
	cdev_del(buttons_cdev);
	//Free even and odd interrupts
	free_irq(17,NULL);
	free_irq(18,NULL);

	//Disable GPIO interrupts
	printk(KERN_DEBUG "GAMEPAD:Disable GPIUO interrupts\n");
	iowrite32(0x0, gpio_int_mem + IEN_OFFSET);
	iowrite32(0x0, gpio_int_mem + EXTIRISE_OFFSET);
	iowrite32(0x0, gpio_int_mem + EXTIFALL_OFFSET);

	printk(KERN_DEBUG "GAMEPAD:Unmap GPIO\n");
	iounmap(gpio_porta_mem);
	iounmap(gpio_portc_mem);
    iounmap(gpio_int_mem);

	//Release memory
	printk(KERN_DEBUG "GAMEPAD:Release memory region\n");
	release_mem_region(GPIO_PA_BASE, 0x24);
	release_mem_region(GPIO_PC_BASE, 0x24);
    release_mem_region(GPIO_INT_BASE, 0x20);

	//Destroy class and device
	printk(KERN_DEBUG "GAMEPAD:Destroy class and device\n");
	device_destroy(cl,devNumber);
	class_destroy(cl);

	//Unregister char device region
	printk(KERN_DEBUG "GAMEPAD:Unregister char device region\n");
	unregister_chrdev_region(devNumber, devCount);
}


/* Functions for using the gamepad from userspace */

// Userspace program opens driver
static int gamepad_open(struct inode *inode,struct file *file){
    if (driverOpen) {
        return -EBUSY;
    }

    try_module_get(THIS_MODULE); // Prevent module unloading while in use
    driverOpen = 1;
    return SUCCESS; //Configuration handled by the module_init
}

// Userspace program closes driver
static int gamepad_release(struct inode *inode, struct file *file){
    driverOpen = 0;
    module_put(THIS_MODULE); // Allow module unloading
    return SUCCESS; //Configuration handled by the module_exit
}

// user program reads from the driver
static ssize_t my_read (struct file *filp, char __user *buff, size_t count, loff_t *offp){
    uint8_t data = ioread8(GPIO_PC_DIN);
    copy_to_user(buff, &data, 8);

    return SUCCESS;
}



//user program writes to the driver
static ssize_t my_write (struct file *filp, const char __user *buff, size_t count, loff_t *offp){
    return SUCCESS; //Not used as we do not want to write to the LEDs on the gamepad
}

static irq_handler_t interrupt_handler(int irq, void *dev_id, struct pt_regs *regs){
    
    // Clear interrupt flags
    iowrite32(0xffff, gpio_int_mem + IFC_OFFSET);
    printk(KERN_DEBUG "GAMEPAD:GPIO Interrupt\n");
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

