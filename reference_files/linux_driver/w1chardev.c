/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <asm/atomic.h>

#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/clk.h>

#include <linux/fs.h>
#include <linux/device.h>

#define XLNX_IOCTL_RESET_BUS 	_IOR('k', 0, int)
#define XLNX_IOCTL_READ_BIT 	_IOR('k', 1, int)
#define XLNX_IOCTL_WRITE_BIT 	_IOW('k', 2, int)
#define XLNX_IOCTL_READ_BYTE 	_IOR('k', 3, int)
#define XLNX_IOCTL_WRITE_BYTE	_IOW('k', 4, int)


/* 1-wire XLNX IP definition */
// Registers offset
#define AXIW1_INST_REG 	0x0
#define AXIW1_CTRL_REG 	0x4
#define AXIW1_IRQE_REG	0x8
#define AXIW1_STAT_REG 	0xC
#define AXIW1_DATA_REG 	0x10
// Instructions
#define AXIW1_READBIT 	0x00000C00
#define AXIW1_WRITEBIT 	0x00000E00
#define AXIW1_READBYTE 	0x00000D00
#define AXIW1_WRITEBYTE	0x00000F00
#define AXIW1_INITPRES 	0x00000800
// Status flag masks
#define AXIW1_DONE 		0x00000001
#define AXIW1_READY		0x00000010
#define AXI_PRESENCE	0x80000000
// Control flag
#define AXIW1_GO 		0x00000001
#define AXI_CLEAR 		0x00000000
#define AXI_RESET 		0x80000000
// Interrupt Enable
#define AXIW1_READY_IRQ_EN 	0x00000010
#define AXIW1_DONE_IRQ_EN 	0x00000001

#define DEVICE_NAME "xlnx_w1"
#define CLASS_NAME "xlnx_w1_class"
static int major_num;
static struct class* w1Class = NULL;
static struct device* w1Device = NULL;

static void __iomem *xlnxw1_base_register;
static int device_in_use = 0;
static wait_queue_head_t wait_queue;
static atomic_t flag;

#define DRIVER_NAME "xlnxw1"

struct xlnxw1_local
{
	struct device *dev;
	int irq;
	void __iomem *base_addr;
};

/* Functions to write and read the W1 IP register */
static inline void xlnxw1_write_register(u8 reg_offset, u32 val)
{
	iowrite32(val, xlnxw1_base_register + reg_offset);
	return;
};

static inline u32 xlnxw1_read_register(u8 reg_offset)
{
	u32 val = 0;
	val = ioread32(xlnxw1_base_register + reg_offset);
	return val;
};

static long xlnxw1_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	u8 val = 0;
	switch (cmd)
	{
	case XLNX_IOCTL_RESET_BUS:
		// Reset 1-wire Axi IP
		xlnxw1_write_register(AXIW1_CTRL_REG, AXI_RESET);
		// Wait for READY signal to be 1 to ensure 1-wire IP is ready
		while((xlnxw1_read_register(AXIW1_STAT_REG) & AXIW1_READY) == 0)
		{
			// Enable the ready signal interrupt
			xlnxw1_write_register(AXIW1_IRQE_REG, AXIW1_READY_IRQ_EN);
			wait_event_interruptible(wait_queue, atomic_read(&flag) != 0);
			atomic_set(&flag, 0);
		}
		
		// Write Initialization command in register 0
		xlnxw1_write_register(AXIW1_INST_REG, AXIW1_INITPRES);
		// Write Go signal and clear control reset signal in register 1
		xlnxw1_write_register(AXIW1_CTRL_REG, AXIW1_GO);
		
		// Wait for Done signal to be 1
		while((xlnxw1_read_register(AXIW1_STAT_REG) & AXIW1_DONE) != 1)
		{
			// Enable the done signal interrupt
			xlnxw1_write_register(AXIW1_IRQE_REG, AXIW1_DONE_IRQ_EN);
			wait_event_interruptible(wait_queue, atomic_read(&flag) != 0);
			atomic_set(&flag, 0);
		}
		
		// Retrieve MSB bit in register 2 to get failure bit
		if ((xlnxw1_read_register(AXIW1_STAT_REG) & AXI_PRESENCE) != 0) {
			val = 1;
		}
		// Clear Go signal in register 1
		xlnxw1_write_register(AXIW1_CTRL_REG, AXI_CLEAR);

		if(copy_to_user((u8 *) arg, &val, sizeof(u8)))
		{
			return -EFAULT;
		}
		break;
	
	case XLNX_IOCTL_READ_BIT:
		// Wait for READY signal to be 1 to ensure 1-wire IP is ready
		while((xlnxw1_read_register(AXIW1_STAT_REG) & AXIW1_READY) == 0)
		{
			// Enable the ready signal interrupt
			xlnxw1_write_register(AXIW1_IRQE_REG, AXIW1_READY_IRQ_EN);
			wait_event_interruptible(wait_queue, atomic_read(&flag) != 0);
			atomic_set(&flag, 0);
		}
		
		// Write read Bit command in register 0
		xlnxw1_write_register(AXIW1_INST_REG, AXIW1_READBIT);
		// Write Go signal and clear control reset signal in register 1
		xlnxw1_write_register(AXIW1_CTRL_REG, AXIW1_GO);
		
		// Wait for Done signal to be 1
		while((xlnxw1_read_register(AXIW1_STAT_REG) & AXIW1_DONE) != 1)
		{
			// Enable the done signal interrupt
			xlnxw1_write_register(AXIW1_IRQE_REG, AXIW1_DONE_IRQ_EN);
			wait_event_interruptible(wait_queue, atomic_read(&flag) != 0);
			atomic_set(&flag, 0);
		}
		
		// Retrieve LSB bit in register 3 to get RX byte
		val = (u8) (xlnxw1_read_register(AXIW1_DATA_REG) & 0x00000001);
		
		// Clear Go signal in register 1
		xlnxw1_write_register(AXIW1_CTRL_REG, AXI_CLEAR);
		
		if(copy_to_user((u8 *) arg, &val, sizeof(u8)))
		{
			return -EFAULT;
		}
		break;

	case XLNX_IOCTL_WRITE_BIT:
		if(copy_from_user(&val, (u8 *) arg, sizeof(u8)))
		{
			return -EFAULT;
		}
		// Wait for READY signal to be 1 to ensure 1-wire IP is ready
		while((xlnxw1_read_register(AXIW1_STAT_REG) & AXIW1_READY) == 0)
		{
			// Enable the ready signal interrupt
			xlnxw1_write_register(AXIW1_IRQE_REG, AXIW1_READY_IRQ_EN);
			wait_event_interruptible(wait_queue, atomic_read(&flag) != 0);
			atomic_set(&flag, 0);
		}
		
		// Write tx Byte command in register 0 with bit to transmit
		xlnxw1_write_register(AXIW1_INST_REG, (AXIW1_WRITEBIT + (val & 0x01)));
		// Write Go signal and clear control reset signal in register 1
		xlnxw1_write_register(AXIW1_CTRL_REG, AXIW1_GO);
		
		// Wait for Done signal to be 1
		while((xlnxw1_read_register(AXIW1_STAT_REG) & AXIW1_DONE) != 1)
		{
			// Enable the done signal interrupt
			xlnxw1_write_register(AXIW1_IRQE_REG, AXIW1_DONE_IRQ_EN);
			wait_event_interruptible(wait_queue, atomic_read(&flag) != 0);
			atomic_set(&flag, 0);
		}
		
		// Clear Go signal in register 1
		xlnxw1_write_register(AXIW1_CTRL_REG, AXI_CLEAR);
		break;

	case XLNX_IOCTL_READ_BYTE:
		// Wait for READY signal to be 1 to ensure 1-wire IP is ready
		while((xlnxw1_read_register(AXIW1_STAT_REG) & AXIW1_READY) == 0)
		{
			// Enable the ready signal interrupt
			xlnxw1_write_register(AXIW1_IRQE_REG, AXIW1_READY_IRQ_EN);
			wait_event_interruptible(wait_queue, atomic_read(&flag) != 0);
			atomic_set(&flag, 0);
		}
		
		// Write read Byte command in register 0
		xlnxw1_write_register(AXIW1_INST_REG, AXIW1_READBYTE);
		// Write Go signal and clear control reset signal in register 1
		xlnxw1_write_register(AXIW1_CTRL_REG, AXIW1_GO);
		
		// Wait for Done signal to be 1
		while((xlnxw1_read_register(AXIW1_STAT_REG) & AXIW1_DONE) != 1)
		{
			// Enable the done signal interrupt
			xlnxw1_write_register(AXIW1_IRQE_REG, AXIW1_DONE_IRQ_EN);
			wait_event_interruptible(wait_queue, atomic_read(&flag) != 0);
			atomic_set(&flag, 0);
		}
		
		// Retrieve LSB bit in register 3 to get RX byte
		val = (u8) (xlnxw1_read_register(AXIW1_DATA_REG) & 0x000000FF);
		
		// Clear Go signal in register 1
		xlnxw1_write_register(AXIW1_CTRL_REG, AXI_CLEAR);
		
		if(copy_to_user((u8 *) arg, &val, sizeof(u8)))
		{
			printk("HERE\n");
			return -EFAULT;
		}
		break;

	case XLNX_IOCTL_WRITE_BYTE:
		if(copy_from_user(&val, (u8 *) arg, sizeof(u8)))
		{
			return -EFAULT;
		}
		// Wait for READY signal to be 1 to ensure 1-wire IP is ready
		while((xlnxw1_read_register(AXIW1_STAT_REG) & AXIW1_READY) == 0)
		{
			// Enable the ready signal interrupt
			xlnxw1_write_register(AXIW1_IRQE_REG, AXIW1_READY_IRQ_EN);
			wait_event_interruptible(wait_queue, atomic_read(&flag) != 0);
			atomic_set(&flag, 0);
		}
		
		// Write tx Byte command in register 0 with bit to transmit
		xlnxw1_write_register(AXIW1_INST_REG, (AXIW1_WRITEBYTE + (val & 0xFF)));
		// Write Go signal and clear control reset signal in register 1
		xlnxw1_write_register(AXIW1_CTRL_REG, AXIW1_GO);
		
		// Wait for Done signal to be 1
		while((xlnxw1_read_register(AXIW1_STAT_REG) & AXIW1_DONE) != 1)
		{
			// Enable the done signal interrupt
			xlnxw1_write_register(AXIW1_IRQE_REG, AXIW1_DONE_IRQ_EN);
			wait_event_interruptible(wait_queue, atomic_read(&flag) != 0);
			atomic_set(&flag, 0);
		}
		
		// Clear Go signal in register 1
		xlnxw1_write_register(AXIW1_CTRL_REG, AXI_CLEAR);
		break;
		
	default:
		return -EINVAL;
	}
	return 0;
}

static int xlnxw1_open(struct inode *inode, struct file *file)
{	
	if (device_in_use) {
		return -EBUSY;
	}
	device_in_use++;
	try_module_get(THIS_MODULE);
	return 0;
}

static int xlnxw1_release(struct inode *inode, struct file *file)
{
	device_in_use--;
	module_put(THIS_MODULE);
	return 0;
}

struct file_operations Fops = {
	.owner 			= THIS_MODULE,
	.unlocked_ioctl = xlnxw1_ioctl,
	.open 			= xlnxw1_open,
	.release		= xlnxw1_release,
};

static irqreturn_t xlnxw1_irq(int irq, void *lp)
{
	// Clear enables in IRQ enable register
	xlnxw1_write_register(AXIW1_IRQE_REG, AXI_CLEAR);
	// Wake up the waiting queue
	atomic_set(&flag, 1);
	wake_up_interruptible(&wait_queue);
	
	return IRQ_HANDLED;
}

static int xlnxw1_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct xlnxw1_local *lp;
	struct clk *clk;
	
	int rc = 0; 

	lp = devm_kzalloc(dev, sizeof(*lp), GFP_KERNEL); 
	if (!lp) { 
		dev_err(dev, "Could not allocate xlnxw1 device\n"); 
		return -ENOMEM; 
	} 

	lp->dev = dev;
	lp->base_addr = devm_platform_ioremap_resource(pdev, 0); 
	if (IS_ERR(lp->base_addr)) {
		dev_err(dev, "Could not allocate resource\n");
		return PTR_ERR(lp->base_addr); 
	} 

	/* Get IRQ for the device */ 
	lp->irq = platform_get_irq(pdev, 0); 
	if (lp->irq < 0) { 
		dev_err(dev, "xlnxw1: Could not find an interrupt. An interrupt is required to use this driver\n"); 
		return lp->irq;
	} 
	
	rc = devm_request_irq(dev, lp->irq, &xlnxw1_irq, IRQF_TRIGGER_HIGH, DRIVER_NAME, lp); 
	if (rc) { 
		dev_err(dev, "xlnxw1: Could not allocate interrupt %d.\n", 
			lp->irq); 
		return rc; 
	} 

	// Initialize wait queue and flag 
	init_waitqueue_head(&wait_queue); 
	atomic_set(&flag, 0); 

	clk = devm_clk_get_enabled(dev, NULL); 
	if(IS_ERR(clk)){ 
		dev_err(dev, "Clock error\n"); 
		return PTR_ERR(clk); 
	}  

	dev_info(dev,"xlnxw1 mapped to 0x%08x, irq=%d\n", 
		(unsigned int __force)lp->base_addr, 
		lp->irq); 
	
	if(ioread32(lp->base_addr + 0x1C)!= 0x10ee4453){ 
		dev_err(dev, "1-Wire IP not detected\n"); 
		return -ENODEV; 
	}

	platform_set_drvdata(pdev, lp); 
	xlnxw1_base_register = lp->base_addr; 
	return 0; 
}

static int xlnxw1_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct xlnxw1_local *lp = dev_get_drvdata(dev);
	iounmap(lp->base_addr);
	dev_set_drvdata(dev, NULL);
	return 0;
}

#ifdef CONFIG_OF
static struct of_device_id xlnxw1_of_match[] = {
	{ .compatible = "xlnx,axi-1wire-host-0.1", },
	{ /* end of list */ },
};
MODULE_DEVICE_TABLE(of, xlnxw1_of_match);
#else
# define xlnxw1_of_match
#endif


static struct platform_driver xlnxw1_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table	= xlnxw1_of_match,
	},
	.probe		= xlnxw1_probe,
	.remove		= xlnxw1_remove,
};

static int __init xlnxw1_init(void)
{
	major_num = register_chrdev(0, DEVICE_NAME, &Fops);
	if (major_num < 0) {
		printk(KERN_ALERT "Failed to register a major number for the 1-wire device.\n");
		return -1;
	}
	printk(KERN_INFO "Registration successful, 1-wire device's major number is %d.\n", major_num);

	w1Class = class_create(CLASS_NAME);
	if (IS_ERR(w1Class))
	{
		printk(KERN_ALERT "Failed to register class for the 1-wire device.\n");
		goto error1;
	}
	printk(KERN_INFO "1-wire class registration successful.\n");

	w1Device = device_create(w1Class, NULL, MKDEV(major_num, 0), NULL, DEVICE_NAME);
	if (IS_ERR(w1Device))
	{
		printk(KERN_ALERT "Failed to create the 1-wire device.\n");
		goto error2;
	}
	printk(KERN_INFO "1-wire device created successfully.\n");

	return platform_driver_register(&xlnxw1_driver);
error2:
	class_destroy(w1Class);
error1:
	unregister_chrdev(major_num,DEVICE_NAME);
	return -1;
}


static void __exit xlnxw1_exit(void)
{
	device_destroy(w1Class, MKDEV(major_num, 0));
	class_unregister(w1Class);
	class_destroy(w1Class);
	unregister_chrdev(major_num, DEVICE_NAME);
	platform_driver_unregister(&xlnxw1_driver);
	printk(KERN_ALERT "1-wire device unregistered.\n");
}

module_init(xlnxw1_init);
module_exit(xlnxw1_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thomas Delev");
MODULE_DESCRIPTION("Character driver to use with AMD 1 Wire IP core version 0.1");