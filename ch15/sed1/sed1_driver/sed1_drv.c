/*
 * ch15/sed1/sed1_drv/sed1_drv.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 15 : Kernel timers, workqueues and more
 ****************************************************************
 * Brief Description:
 *
 * For details, pl refer the book, Ch 15.
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
#include <linux/timer.h>
#include <linux/delay.h>
#include <asm/atomic.h>

// copy_[to|from]_user()
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 11, 0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

#include "../sed_common.h"
#include "../../../convenient.h"

#define DRVNAME  "sed1_drv"
#define TIMER_EXPIRE_MS		1

MODULE_DESCRIPTION("sed1: simple encrypt-decrypt driver; demo misc driver for kernel timers (and ioctl)");
MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/* Module parameters */
static int make_it_fail;
module_param(make_it_fail, int, 0660);
MODULE_PARM_DESC(make_it_fail,
"Deliberately ensure that the kernel timeout occurs before proessing completes (default=0)");

/*
 * The driver 'context' (or private) data structure;
 * all relevant 'state info' reg the driver is here.
 */
struct stMyCtx {
	struct device *dev;
	unsigned int fake;
	atomic_t timed_out;
	struct timer_list timr;
};

//static struct timer_list timr;


/*-------------- For registration with a bus driver ---------------------*/
// Here, we shall register ourselves with the 'platform' bus

static int platdev_probe(struct platform_device *pdev);
static int platdev_remove(struct platform_device *pdev);
static void plat0_release(struct device *dev);

/* Define our pseudo platform device */
static struct platform_device plat0 = {
#if 1				/* Make 1 to test using different names in the platform device and driver;
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


static void timesup(struct timer_list *timer)
{
	//struct stMyCtx *priv = plat0.dev.platform_data;
	struct stMyCtx *priv = from_timer(priv, timer, timr);

	atomic_set(&priv->timed_out, 1);
	pr_notice("*** Timer expired! ***\n");
    PRINT_CTX();
}

#define CRYPT_OFFSET	63

/*
 * TOO (Theory Of Operation):
 * The inverse of the XOR operation is the XOR !
 * a ^ x = (a ^ x) ^ x
 * So, here in the decrypt routine, we perform the work shown above.
 *
 * @kd    : encrypted content
 * @kdret : will be set to the decrypted content
 */
static void decrypt_payload(struct sed_ds *kd, struct sed_ds *kdret)
{
	int i;
	ktime_t t1, t2; // a s64 qty
	struct stMyCtx *priv = plat0.dev.platform_data;

	pr_debug("starting timer + processing now ...\n");

	/* Start - the timer; set it to expire in TIMER_EXPIRE_MS ms */
	mod_timer(&priv->timr, jiffies + msecs_to_jiffies(TIMER_EXPIRE_MS));

	t1 = ktime_get_real_ns();

	// perform the actual processing on the payload
	memcpy(kdret, kd, sizeof(struct sed_ds));
	for (i=0; i<kd->len; i++) {
		//kdret->data[i] = kd->data[i] - CRYPT_OFFSET;
		kdret->data[i] = kd->data[i] ^ CRYPT_OFFSET;
	}
	kdret->len = kd->len;

	if (make_it_fail == 1) {
	// ok with msleep()+1 or, via mdelay(), with not less than mdelay()+10 !
#if 0
		mdelay(TIMER_EXPIRE_MS+10);
#else
		msleep(TIMER_EXPIRE_MS+1);
#endif
	}
	t2 = ktime_get_real_ns();

	// done; cancel the timeout
	del_timer(&priv->timr);
	SHOW_DELTA(t1, t2);
	pr_debug("processing complete, timer deleted\n");
}

/*
 * TOO (Theory Of Operation):
 * The inverse of the XOR operation is the XOR !
 * a ^ x = (a ^ x) ^ x
 * So, here in the encrypt routine, we perform the first part, the (a ^ x)
 *
 * @kd    : cleartext content
 * @kdret : will be set to the encrypted content
 */
static void encrypt_payload(struct sed_ds *kd, struct sed_ds *kdret)
{
	int i;
	ktime_t t1, t2; // a s64 qty
	struct stMyCtx *priv = plat0.dev.platform_data;

	pr_debug("starting timer + processing now ...\n");
//print_hex_dump_bytes("kd: ", DUMP_PREFIX_OFFSET, kd, sizeof(struct sed_ds));

	/* Start - the timer; set it to expire in TIMER_EXPIRE_MS ms */
	mod_timer(&priv->timr, jiffies + msecs_to_jiffies(TIMER_EXPIRE_MS));

	t1 = ktime_get_real_ns();

	// perform the actual processing on the payload
	memcpy(kdret, kd, sizeof(struct sed_ds));
	for (i=0; i<kd->len; i++) {
		//kdret->data[i] = kd->data[i] + CRYPT_OFFSET;
		//pr_debug("0x%x ^ %d\n", 	kd->data[i] , CRYPT_OFFSET);
		kdret->data[i] ^= CRYPT_OFFSET;
	}
	kdret->len = kd->len;

	if (make_it_fail == 1) {
	// ok with msleep()+1 or, via mdelay(), with not less than mdelay()+10 !
#if 0
		mdelay(TIMER_EXPIRE_MS+10);
#else
		msleep(TIMER_EXPIRE_MS+1);
#endif
	}
	t2 = ktime_get_real_ns();

	// done; cancel the timeout
	del_timer(&priv->timr);
	SHOW_DELTA(t1, t2);
	pr_debug("processing complete, timer deleted\n");
}


#define WORK_IS_ENCRYPT		1
#define WORK_IS_DECRYPT		2

/*
 * TOO (Theory Of Operation):
 * The inverse of the XOR operation is the XOR !
 * a ^ x = (a ^ x) ^ x
 * So, here in the encrypt routine, we perform the first part, the (a ^ x)
 *
 * @kd    : cleartext content
 * @kdret : will be set to the encrypted content
 */
static void encrypt_decrypt_payload(int work, struct sed_ds *kd, struct sed_ds *kdret)
{
    int i;
    ktime_t t1, t2; // a s64 qty
    struct stMyCtx *priv = plat0.dev.platform_data;

    pr_debug("starting timer + processing now ...\n");
//print_hex_dump_bytes("kd: ", DUMP_PREFIX_OFFSET, kd, sizeof(struct sed_ds));

    /* Start - the timer; set it to expire in TIMER_EXPIRE_MS ms */
    mod_timer(&priv->timr, jiffies + msecs_to_jiffies(TIMER_EXPIRE_MS));

    t1 = ktime_get_real_ns();

    // perform the actual processing on the payload
    memcpy(kdret, kd, sizeof(struct sed_ds));
	if (work == WORK_IS_ENCRYPT) {
		for (i=0; i<kd->len; i++) {
			 //kdret->data[i] = kd->data[i] + 1;
			 //pr_debug("0x%x ^ %d\n",   kd->data[i] , CRYPT_OFFSET);
			 kdret->data[i] ^= CRYPT_OFFSET;
		  }
	} else if (work == WORK_IS_DECRYPT) {
		  for (i=0; i<kd->len; i++) {
			  kdret->data[i] ^= CRYPT_OFFSET;
			  //kdret->data[i] = kd->data[i] - 1;
			  //pr_debug("0x%x ^ %d\n",   kd->data[i] , CRYPT_OFFSET);
		  }
	}
    kdret->len = kd->len;

    if (make_it_fail == 1) {
    // ok with msleep()+1 or, via mdelay(), with not less than mdelay()+10 !
#if 0
        mdelay(TIMER_EXPIRE_MS+10);
#else
        msleep(TIMER_EXPIRE_MS+1);
#endif
    }
    t2 = ktime_get_real_ns();

	// done; cancel the timeout
	del_timer(&priv->timr);
	SHOW_DELTA(t1, t2);
	pr_debug("processing complete, timeout cancelled\n");
}



/*
 * process_it - our "processing" work is kicked off here
 */
static void process_it(struct sed_ds *kd, struct sed_ds *kdret)
{
//print_hex_dump_bytes("kd: ", DUMP_PREFIX_OFFSET, kd, sizeof(struct sed_ds));
	switch(kd->data_xform) {
	case XF_NONE:
		pr_debug("data transform type: XF_NONE\n");
		// nothing to do
		break;
	case XF_ENCRYPT:
		pr_debug("data transform type: XF_ENCRYPT\n");
		encrypt_decrypt_payload(WORK_IS_ENCRYPT, kd, kdret);
		//encrypt_payload(kd, kdret);
		break;
	case XF_DECRYPT:
		pr_debug("data transform type: XF_DECRYPT\n");
		encrypt_decrypt_payload(WORK_IS_DECRYPT, kd, kdret);
		//decrypt_payload(kd, kdret);
		break;
	}
}

/* 
 * The key method - the ioctl - for our demo driver; note how we take into
 * account the fact that the ioctl's signtaure changed from 2.6.36 (as the
 * BKL was finally removed).
 *
 * CAREFUL!
 * What if two or more threads (or processes) concurrentyl invoke this driver
 * ioctl() method? There will be concurrency issues! ... which we haven't taken
 * care of here... This, and a lot more, will be discussed and detailed in this
 * book's last two chapters on Kernel Synchronization, a very real and important
 * topic.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
static long ioctl_miscdrv(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int ioctl_miscdrv(struct inode *ino, struct file *filp, unsigned int cmd,
		     unsigned long arg)
#endif
{
	int retval = 0;
	struct sed_ds *kd, *kdret;
	struct stMyCtx *priv = plat0.dev.platform_data;

	//pr_debug("In ioctl method, cmd=%d\n", _IOC_NR(cmd));

	/* Verify stuff: is the ioctl's for us? etc.. */
	if (_IOC_TYPE(cmd) != IOCTL_LLKD_SED_MAGIC) {
		pr_warn("ioctl fail; magic # mismatch\n");
		return -ENOTTY;
	}
	if (_IOC_NR(cmd) > IOCTL_LLKD_SED_MAXIOCTL) {
		pr_warn("ioctl fail; invalid cmd?\n");
		return -ENOTTY;
	}

	kd = kzalloc(sizeof(struct sed_ds), GFP_KERNEL);
	if (!kd)
		return -ENOMEM;
	kdret = kzalloc(sizeof(struct sed_ds), GFP_KERNEL);
	if (!kdret)
		return -ENOMEM;

	switch (cmd) {
	/*case IOCTL_LLKD_SED_IOCRESET:
		pr_debug("In ioctl cmd option: IOCTL_LLKD_SED_IOCRESET\n");
		break;
	 */
	case IOCTL_LLKD_SED_IOC_ENCRYPT_MSG:
	case IOCTL_LLKD_SED_IOC_DECRYPT_MSG:
		pr_debug("In ioctl cmd option: %s\narg=0x%lx\n",
			(cmd == IOCTL_LLKD_SED_IOC_ENCRYPT_MSG ? "encrypt" : "decrypt"), arg);
#if 0
		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
#endif
		if (copy_from_user(kd, (struct sed_ds *)arg, sizeof(struct sed_ds))) {
			pr_warn("copy_from_user() failed\n");
	/* TODO : use goto */
			kfree(kdret);
			kfree(kd);
			return -EFAULT;
		}
		pr_debug("xform=%d, len=%d\n", kd->data_xform, kd->len);
		print_hex_dump_bytes("payload: ", DUMP_PREFIX_OFFSET, kd->data, kd->len);
		process_it(kd, kdret);
		if (atomic_read(&priv->timed_out) == 1) {
			kdret->timed_out = 1;
			pr_debug("** timed out **\n");
		}
		print_hex_dump_bytes("ret payload: ", DUMP_PREFIX_OFFSET, kdret->data, kdret->len);

		// write back processed payload to the user space process
		if (copy_to_user((struct sed_ds *)arg, (struct sed_ds *)kdret, sizeof(struct sed_ds))) {
			pr_warn("copy_to_user() failed\n");
			kfree(kdret);
			kfree(kd);
			return -EFAULT;
		}

		break;
#if 0
	case IOCTL_LLKD_SED_IOC_DECRYPT_MSG:
		pr_debug("In ioctl cmd option: IOCTL_LLKD_SED_IOC_DECRYPT_MSG\n");
		if (copy_from_user(kd, (struct sed_ds *)arg, sizeof(struct sed_ds))) {
			pr_warn("copy_from_user() failed\n");
	/* TODO : use goto */
			kfree(kdret);
			kfree(kd);
			return -EFAULT;
		}
		pr_debug("xform=%d, len=%d\n", kd->data_xform, kd->len);
		print_hex_dump_bytes("payload: ", DUMP_PREFIX_OFFSET, kd->data, kd->len);
		process_it(kd, kdret);
		if (atomic_read(&priv->timed_out) == 1) {
			kdret->timed_out = 1;
			pr_debug("** timed out **\n");
		}
		print_hex_dump_bytes("ret payload: ", DUMP_PREFIX_OFFSET, kdret->data, kdret->len);
		//print_hex_dump_bytes("ret payload: ", DUMP_PREFIX_OFFSET, kdret, sizeof(struct sed_ds));

		// write back processed payload to the user space process
		if (copy_to_user((struct sed_ds *)arg, (struct sed_ds *)kdret, sizeof(struct sed_ds))) {
			pr_warn("copy_to_user() failed\n");
			kfree(kdret);
			kfree(kd);
			return -EFAULT;
		}

		break;
#endif
	default:
		kfree(kdret);
		kfree(kd);
		return -ENOTTY;
	}

	kfree(kdret);
	kfree(kd);
	return retval;
}

/*----------- 'misc' methods -------------*/
static int open_miscdrv(struct inode *inode, struct file *filp)
{
	PRINT_CTX();		// displays process (or atomic) context info
	pr_info("opening \"%s\" now\n", filp->f_path.dentry->d_iname);

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
	pr_info("closing \"%s\"\n", filp->f_path.dentry->d_iname);
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

	priv->fake = 100;
	dev_dbg(dev, "platform driver: probe method invoked\n");
	return 0;
}

static int platdev_remove(struct platform_device *pdev)
{
	struct stMyCtx *priv = dev_get_platdata(&pdev->dev);
	struct device *dev = &pdev->dev;

	dev_dbg(dev, "platform driver:remove method invoked\n\n");
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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	.unlocked_ioctl = ioctl_miscdrv,	// use the 'unlocked' version
#else
	.ioctl = ioctl_miscdrv,   // 'old' way
#endif
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

static int __init ktimer_drv_init(void)
{
	int ret = 0;
	struct device *dev;
	struct stMyCtx *priv = NULL;

	pr_info("%s: Initializing platform demo driver now...\n", DRVNAME);

	//--- Register with misc kernel framework
	ret = misc_register(&llkd_miscdev);
	if (ret) {
		pr_notice("misc device registration failed, aborting\n");
		return ret;
	}
	dev = llkd_miscdev.this_device;

	dev_info(dev, "LLKD misc driver (major # 10) registered, minor# = %d,"
		 " dev node is /dev/%s\n", llkd_miscdev.minor, DRVNAME);

	priv = devm_kzalloc(dev, sizeof(struct stMyCtx), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	priv->dev = llkd_miscdev.this_device;
	//ATOMIC_INIT(&priv->timed_out);
	plat0.dev.platform_data = priv;	// convenient to access our private structure later

#if 0
	//--- Add platform device(s) and register with platform bus
	/* platform_add_devices() is a wrapper over platform_device_register() */
	ret = platform_add_devices(platdemo_platform_devices,
				   ARRAY_SIZE(platdemo_platform_devices));
	if (ret) {
		pr_alert("%s: platform_add_devices failed!\n", DRVNAME);
		goto out_fail_pad;
	}

	ret = platform_driver_register(&platdrv);
	if (ret) {
		pr_alert("%s: platform_driver_register failed!\n", DRVNAME);
		goto out_fail_pdr;
	}
	/* Successful platform_driver_register() will cause the registered 'probe'
	 * method to be invoked now..
	 */
#endif

	// Initialize our kernel timer
//	memset(priv->timr, 0, sizeof(struct timer_list));
	timer_setup(&priv->timr, timesup, 0);
	
	pr_info("init done (make_it_fail is %s)\n", make_it_fail == 1 ? "on" : "off");
	dev_dbg(dev, "loaded.\n");
	return ret;

 out_fail_pdr:
	platform_device_unregister(&plat0);
 out_fail_pad:
	return ret;
}

static void __exit ktimer_drv_exit(void)
{
	struct stMyCtx *priv = plat0.dev.platform_data;

	dev_dbg(priv->dev, "unloading\n");
	del_timer_sync(&priv->timr);
	misc_deregister(&llkd_miscdev);
#if 0
	platform_driver_unregister(&platdrv);
	platform_device_unregister(&plat0);
#endif
}

module_init(ktimer_drv_init);
module_exit(ktimer_drv_exit);
