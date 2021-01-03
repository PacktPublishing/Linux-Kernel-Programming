/*
 * solutions_to_assgn/ch12/misc_plat.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 12 : Writing a simple misc character device driver
 ****************************************************************
 * Brief Description:
 * For convenience, we repeat the 'Question' here:
 * 4. (A bit more advanced). Write a "proper" LDM-based driver. Meaning this:
 * the 'misc' drivers covered here did register with the kernel's 'misc'
 * framework, but simply, implicitly, used the raw character interface as the
 * bus. The LDM prefers that a driver must register with a kernel framework and
 * a bus driver. Hence, write a 'demo' driver that registers itself with the
 * kernel's misc framework and with the platform bus. This will involve
 * creating a fake platform device as well.
 *
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

// copy_[to|from]_user()
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 11, 0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif
#include "../../convenient.h"

#define DRVNAME  "misc_plat"

MODULE_DESCRIPTION("Simple demo misc driver on the platform bus");
MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/*
 * The driver 'context' (or private) data structure;
 * all relevant 'state info' reg the driver is here.
 */
struct stMyCtx {
	struct device *dev;
	unsigned int data_xform;
};

/*----------- 'misc' methods -------------*/
static int open_miscdrv(struct inode *inode, struct file *filp)
{
	char *buf = kzalloc(PATH_MAX, GFP_KERNEL);

	if (unlikely(!buf))
		return -ENOMEM;

	PRINT_CTX();		// displays process (or atomic) context info
	pr_info("opening \"%s\" now\n", file_path(filp, buf, PATH_MAX));
	kfree(buf);

	return nonseekable_open(inode, filp);
}

static ssize_t read_miscdrv(struct file *filp, char __user *ubuf, size_t count, loff_t *off)
{
	pr_info("to read %zd bytes\n", count);
	return count;
}

static ssize_t write_miscdrv(struct file *filp, const char __user *ubuf,
			     size_t count, loff_t *off)
{
	pr_info("to write %zd bytes\n", count);
	return count;
}

static int close_miscdrv(struct inode *inode, struct file *filp)
{
	char *buf = kzalloc(PATH_MAX, GFP_KERNEL);

	if (unlikely(!buf))
		return -ENOMEM;

	pr_info("closing \"%s\" now\n", file_path(filp, buf, PATH_MAX));
	kfree(buf);

	return 0;
}

/*----------- platform bus methods -------------*/
static int platdev_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct stMyCtx *priv = dev_get_platdata(&pdev->dev);
	/* could have done the above directly with:
	 *  struct stMyCtx *priv = pdev->dev.platform_data;
	 * .. but using the kernel helper is recommended
	 */

	priv->data_xform = 100;
	dev_dbg(dev, "platform driver: probe method invoked\n");
	return 0;
}

static int platdev_remove(struct platform_device *pdev)
{
	struct stMyCtx *priv = dev_get_platdata(&pdev->dev);
	struct device *dev = &pdev->dev;

	dev_dbg(dev, "platform driver:remove method invoked\n data_xform=%d\n",
		priv->data_xform);
	return 0;
}

static void plat0_release(struct device *dev)
{
	dev_dbg(dev, "platform device: release method invoked\n");
}

/*-------------- For registration with a kernel framework ---------------------*/
// Here, we shall register ourselves with the 'misc' kernel framework

/* The driver 'functionality' is encoded via the fops */
static const struct file_operations llkd_misc_fops = {
	.open = open_miscdrv,
	.read = read_miscdrv,
	.write = write_miscdrv,
	.llseek = no_llseek,	// dummy, we don't support lseek(2)
	.release = close_miscdrv,
	/* As you learn more reg device drivers, you'll realize that the
	 * ioctl() would be a very useful method here. As an exercise,
	 * implement an ioctl method; when issued with the 'GETSTATS' 'command',
	 * it should return the statistics (tx, rx, errors) to the calling app.
	 * Refer to our online material "User-Kernel Communication Pathways" for
	 * the details oh how to use the ioctl(), etc.
	 */
};

static struct miscdevice llkd_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,	/* kernel dynamically assigns a free minor# */
	.name = DRVNAME,	/* when misc_register() is invoked, the kernel
				 * will auto-create device file as /dev/llkd_miscdrv_rdwr;
				 *  also populated within /sys/class/misc/ and /sys/devices/virtual/misc/ */
	.mode = 0666,		/* ... dev node perms set as specified here */
	.fops = &llkd_misc_fops,	/* connect to this driver's 'functionality' */
};

/*-------------- For registration with a bus driver ---------------------*/
// Here, we shall register ourselves with the 'platform' bus

/* Define our pseudo platform device */
static struct platform_device plat0 = {
#if 1			/* Make 1 to test using different names in the platform device and driver;
				 * We find that the driver core then can't bind them, and the probe method
				 * is never invoked...
				 */
	.name = "plat",
	/* But then again, we can use the 'driver_override' member to force
	 * a match to the driver's name! Then it still works..
	 */
	.driver_override = DRVNAME,	/* Driver name to force a match to! */
#else
	.name = DRVNAME,
#endif
	.id = 0,
	.dev = {
		.platform_data = NULL,
		/* (the older way):
		   Plug in the device's resources here: memory ranges/IRQs/
		   IOports/DMA channels/clocks/etc + optionally 'data'
		   (typically a private structure).
		   The new way  - on platforms that support it - is of course
		   to use the Device Tree (ARM/PPC) / ACPI tables (x86)
		   to specify both the 'data' and platform-specific resources
		 */
		.release = plat0_release,
		},
};

static struct platform_device *platdemo_platform_devices[] __initdata = {
	&plat0,
};

/* Define our platform driver */
static struct platform_driver platdrv = {
	.probe = platdev_probe,
	.remove = platdev_remove,
	.driver = {
		   .name = DRVNAME,	// matches platform device name
		   .owner = THIS_MODULE,
		   },
};

static int __init misc_plat_init(void)
{
	int ret = 0;
	struct device *dev;
	struct stMyCtx *priv = NULL;

	pr_info("Initializing platform demo driver now...\n");

	//--- Register with misc kernel framework
	ret = misc_register(&llkd_miscdev);
	if (ret) {
		pr_notice("misc device registration failed, aborting\n");
		return ret;
	}
	dev = llkd_miscdev.this_device;

	dev_info(dev, "LLKD misc driver (major # 10) registered, minor# = %d,"
		 " dev node is %s\n", llkd_miscdev.minor, DRVNAME);

	priv = devm_kzalloc(dev, sizeof(struct stMyCtx), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	priv->dev = llkd_miscdev.this_device;
	plat0.dev.platform_data = priv;	// convenient to access later

	//--- Add platform device(s) and register with platform bus
	/* platform_add_devices() is a wrapper over platform_device_register() */
	ret = platform_add_devices(platdemo_platform_devices,
				   ARRAY_SIZE(platdemo_platform_devices));
	if (ret) {
		pr_alert("platform_add_devices failed!\n");
		goto out_fail_pad;
	}

	ret = platform_driver_register(&platdrv);
	if (ret) {
		pr_alert("platform_driver_register failed!\n");
		goto out_fail_pdr;
	}
	/* Successful platform_driver_register() will cause the registered 'probe'
	 * method to be invoked now..
	 */
	dev_dbg(&plat0.dev, "loaded.\n");
	return ret;

 out_fail_pdr:
	platform_device_unregister(&plat0);
 out_fail_pad:
	return ret;
}

static void __exit misc_plat_exit(void)
{
	struct device *dev = &plat0.dev;
	struct stMyCtx *priv = plat0.dev.platform_data;

	dev_dbg(dev, "fyi, data_xform=%d; unloading\n", priv->data_xform);
	platform_driver_unregister(&platdrv);
	platform_device_unregister(&plat0);
}

module_init(misc_plat_init);
module_exit(misc_plat_exit);
