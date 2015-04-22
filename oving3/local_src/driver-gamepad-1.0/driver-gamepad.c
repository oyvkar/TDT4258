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
#include <linux/signal.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/kdev_t.h>
#include <linux/moduleparam.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/siginfo.h>

#include "efm32gg.h"
#include "driver-gamepad.h"


#define SUCCESS 0
dev_t devNumber;
unsigned int devCount = 1;
struct cdev *buttons_cdev;
struct class *cl;
void __iomem *gpio_porta_mem;
void __iomem *gpio_portc_mem;
void __iomem *gpio_int_mem;
static int driverOpen = 0;
static bool buttonState[8];
char buttons[100];
char *msg_ptr;
char *btn_ptr;
struct fasync_struct* async;

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.llseek = noop_llseek,
	.read = gp_read,
    .write = gp_write,
	.open = gp_open,
	.release = gp_release,
    .fasync = gp_fasync
};


// Redirects to fsync_helper
static int gp_fasync(int fd, struct file *filp, int mode) {
    return fasync_helper(fd, filp, mode, &async);
}


// Called on module load
static int __init gamepad_driver_init(void)
{
	printk("Loding gamepad driver\n");

	// Create chardevice and device node
    if( alloc_chrdev_region(&devNumber, 0, devCount, "gamepad") < 0){
		printk(KERN_ERR "GAMEPAD: Character device region allocation FAILED, returning.\n");
		return -1;
	}
	cl = class_create(THIS_MODULE, "gamepad");
	device_create(cl,NULL,devNumber, NULL, "gamepad");
	

	//Request memory region access for GPIO functions and port C, and check if the driver is in use by other processes
	if( request_mem_region(GPIO_PC_BASE, 0x24, "GPIO_port_c") == 0)
	{
		printk(KERN_ERR "GAMEPAD: Port C memory request failed\n");
		return -1;
	}
	if (request_mem_region(GPIO_INT_BASE, 0x10, "GPIO_int") == 0) 
    {
        printk(KERN_ERR "GAMEPAD: GPIO Interrupt memory request failed\n");
        return -1;
    }

	// Remap to virtual addresses
	gpio_portc_mem = ioremap_nocache(GPIO_PC_BASE, 0x24);
	if(gpio_portc_mem == 0)
	{
		printk(KERN_ERR "GAMEPAD: Port C remap failed\n");
		return -1;
	}
    gpio_int_mem = ioremap_nocache(GPIO_INT_BASE, 0x10);
    if(gpio_int_mem == 0) {
        printk(KERN_ERR "GAMEPAD: GPIO Interrupt remap failed");
        return -1;
    }

    
	// Set Port C pin 0-7 as input
    iowrite32(0x33333333,   gpio_portc_mem + MODEL_OFFSET );
    // Set internal pullup
    iowrite32(0xff, 	gpio_portc_mem + DOUT_OFFSET);

    // Setup GPIO IRQ handler, 17 and 18 are odd and even interrupts
	if(request_irq(17,(irq_handler_t) interrupt_handler, 0, "gamepad", NULL) < 0)
	{
		printk(KERN_ERR "GAMEPAD: IRQ 1 request FAILED, returning \n");
		return -1;
	}
    if(request_irq(18, (irq_handler_t) interrupt_handler, 0, "gamepad", NULL) < 0)
	{
		printk(KERN_ERR "GAMEPAD: IRQ 2 request FAILED, returning \n");
		return -1;
	}

    iowrite32(0xFF, gpio_int_mem + IFC_OFFSET); //Clears interrupt flags   
    
	//Activate driver and register allocations
    buttons_cdev = cdev_alloc();
	buttons_cdev->owner = THIS_MODULE;
	buttons_cdev->ops = &fops;
	if(cdev_add(buttons_cdev,devNumber, devCount) < 0)
	{
		printk(KERN_ERR "GAMEPAD: Char device activation failed, returning\n");
		return -1;
	}


	return 0;
}

// Run on module unload
static void __exit gamepad_driver_cleanup(void)
{
	//Deactivate driver
	cdev_del(buttons_cdev);

    //Free even and odd interrupts
	free_irq(17,NULL);
	free_irq(18,NULL);

    // Unmap virtual memory
	iounmap(gpio_portc_mem);
    iounmap(gpio_int_mem);

	//Release memory
	release_mem_region(GPIO_PC_BASE, 0x24);
    release_mem_region(GPIO_INT_BASE, 0x10);

	//Destroy class and device
	device_destroy(cl,devNumber);
	class_destroy(cl);

	//Unregister char device region
	unregister_chrdev_region(devNumber, devCount);
}


// Userspace program opens driver
static int gp_open(struct inode *inode,struct file *file){
    // Allow only one userspace program to access the driver at the same time
    if (driverOpen)
        return -EBUSY;

    try_module_get(THIS_MODULE);                                // Prevent module unloading while in use
    iowrite32(0x22222222,   gpio_int_mem + EXTIPSELL_OFFSET);   // Enable interrupt on port C
    iowrite32(0xff, gpio_int_mem + EXTIRISE_OFFSET);            // Enable interrupt on rising edge
    iowrite32(0xff, gpio_int_mem + EXTIFALL_OFFSET);            // Enable interrupt on falling edge
    iowrite32(0xff, gpio_int_mem + IEN_OFFSET);                 // Enable interrupt
    
    driverOpen++;                                               // Increment open count 
    return SUCCESS; 
}

// Userspace program closes driver
static int gp_release(struct inode *inode, struct file *file){

    iowrite32(0x0, gpio_int_mem + EXTIRISE_OFFSET);     // Disable rising edge interrupts
	iowrite32(0x0, gpio_int_mem + EXTIFALL_OFFSET);     // Disable falling edge interrupts
	iowrite32(0x0, gpio_int_mem + IEN_OFFSET);          // Disable GPIO interrupts when not in use

    driverOpen--;                                       // Decrement open count
    module_put(THIS_MODULE);                            // Allow module unloading
    return SUCCESS; 
}

// user program reads from the driver
static ssize_t gp_read (struct file *filp, char __user *buffer, size_t length, loff_t *offp){
    
   int bytes_read = 0;      // We want to return bytes written to buffer    

   // Write '\0' to buffer and return 1 if no new data is ready
   if (*msg_ptr == 0) {
       msg_ptr = &buttons;
       btn_ptr = &buttons;
       put_user('\0', buffer);
       return 1;
   }

   // Copy data into userspace buffer. 
   // Stops at end of data or buffer full
   while (length && *msg_ptr)  {
         put_user(*msg_ptr, buffer++);
         *msg_ptr = '\0';
         msg_ptr++;
         length--;
         bytes_read++;
   }

   return bytes_read;
}

// Fill buffer with changes in button state
static void button_map(void) {
    uint8_t data;

    // Lazy implementation of a ringbuffer
    if (btn_ptr > (buttons + 60)) {
        btn_ptr = buttons;
        msg_ptr = buttons;
    }

    data = ~ioread8(gpio_portc_mem + DIN_OFFSET);
    int i;
    for (i = 0; i < 8; i++) {
        if( (data & (1 << i)) != buttonState[i]) {
            sprintf(btn_ptr,"SW%i: %i\n", i+1, !buttonState[i]);
            buttonState[i] = !buttonState[i];
            btn_ptr += 7;
        }
    }
}

// User program writes to the driver
// We do not controll any LEDs.
static ssize_t gp_write (struct file *filp, const char __user *buff, size_t count, loff_t *offp){
    return SUCCESS; 
}


static irq_handler_t interrupt_handler(int irq, void *dev_id, struct pt_regs *regs){
    iowrite32(0xFF, gpio_int_mem + IFC_OFFSET); // Clear interrupt flags
    button_map();                               // Map changes in button to output buffer

    if (async)                                  // Signal any userspace process using FASYNC 
        kill_fasync(&async, SIGIO, POLL_IN);   

    return (irq_handler_t) IRQ_HANDLED; 
}

module_init(gamepad_driver_init);
module_exit(gamepad_driver_cleanup);

MODULE_AUTHOR("TDT4258 Group 22");
MODULE_DESCRIPTION("Module for controlling a gamepad provided in the course TDT4258 at NTNU");
MODULE_LICENSE("GPL");

