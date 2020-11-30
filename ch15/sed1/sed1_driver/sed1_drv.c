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
 * From: Ch 15 : Timers, kernel threads and more
 ****************************************************************
 * Brief Description:
 *
 * sed = simple encrypt decrypt (!)
 * In this 'driver', we have setup an interesting (though extremely trivial and
 * simplistic) message encrypt/decrypt facility. The idea is this: a user mode
 * app (it's in ../userapp_sed), opens this misc character driver's device file
 * (it's /dev/sed1_drv) and issues an ioctl(2) upon it. The ioctl() call passes
 * a data structure that encapsulates the data being passed, it's length, the
 * operation (or "transform") to perform upon it, and a timed_out field (to
 * figure out if it failed due to missing it's deadline).
 * The valid ops are :
 *  encrypt : XF_ENCRYPT
 *  decrypt : XF_DECRYPT
 * Associated with the operation is a deadline; it's defined as 1 millisecond.
 * If the op takes longer, a kernel timer we've setup will expire; it will set
 * the context structure's timed_out member to 1 signifying failure. We have
 * the ability for the user mode app to receive all the details and interpret
 * them.
 * So, in a nutshell, the whole idea here is to demo using a kernel timer to
 * timeout an operation.
 * (FYI, though we certainly could, we don't use the dev_<foo>() printk's as
 * would usually be appropriate in a driver, we simply stick with the regular
 * pr_<foo>() routines).
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

#define DRVNAME			"sed1_drv"
#define TIMER_EXPIRE_MS		1

MODULE_DESCRIPTION
("sed1: simple encrypt-decrypt driver; demo misc driver for kernel timers (and ioctl)");
MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/* Module parameters */
static int make_it_fail;
module_param(make_it_fail, int, 0660);
MODULE_PARM_DESC(make_it_fail,
"Deliberately ensure that the kernel timeout occurs before processing completes (default=0)");

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
static struct stMyCtx *gpriv;

static void timesup(struct timer_list *timer)
{
	struct stMyCtx *priv = from_timer(priv, timer, timr);

	atomic_set(&priv->timed_out, 1);
	pr_notice("*** Timer expired! ***\n");
	PRINT_CTX();
}

#define WORK_IS_ENCRYPT		 1
#define WORK_IS_DECRYPT		 2
#define CRYPT_OFFSET		63

/*
 * TOO (Theory Of Operation):
 * The inverse of the XOR operation is the XOR !
 * a ^ x = (a ^ x) ^ x
 * So, here in the encrypt routine, we perform the first part, the (a ^ x)
 *
 * @work  : one of WORK_IS_ENCRYPT or WORK_IS_DECRYPT
 * @kd    : cleartext content
 * @kdret : will be set to the en|de-crypted content
 */
static void encrypt_decrypt_payload(int work, struct sed_ds *kd, struct sed_ds *kdret)
{
	int i;
	ktime_t t1, t2;		// a s64 qty
	struct stMyCtx *priv = gpriv;

	pr_debug("starting timer + processing now ...\n");
	//print_hex_dump_bytes("kd: ", DUMP_PREFIX_OFFSET, kd, sizeof(struct sed_ds));

	/* Start - the timer; set it to expire in TIMER_EXPIRE_MS ms */
	mod_timer(&priv->timr, jiffies + msecs_to_jiffies(TIMER_EXPIRE_MS));

	t1 = ktime_get_real_ns();

	// perform the actual processing on the payload
	memcpy(kdret, kd, sizeof(struct sed_ds));
	if (work == WORK_IS_ENCRYPT) {
		for (i = 0; i < kd->len; i++) {
			kdret->data[i] ^= CRYPT_OFFSET;
			kdret->data[i] += CRYPT_OFFSET;
		}
	} else if (work == WORK_IS_DECRYPT) {
		for (i = 0; i < kd->len; i++) {
			kdret->data[i] -= CRYPT_OFFSET;
			kdret->data[i] ^= CRYPT_OFFSET;
		}
	}
	kdret->len = kd->len;
	// work done!

	if (make_it_fail == 1) {
	// ok with msleep(1ms)+1; via mdelay(), with not less than mdelay(1ms)+10 !
#if 1
		msleep(TIMER_EXPIRE_MS + 1);
#else
		mdelay(TIMER_EXPIRE_MS + 10);
#endif
	}
	t2 = ktime_get_real_ns();

	// work done, cancel the timeout
	if (del_timer(&priv->timr) == 0)
		pr_debug("cancelled the timer while it's inactive! (deadline missed?)\n");
	else
		pr_debug("processing complete, timeout cancelled\n");

	SHOW_DELTA(t2, t1);
}

/*
 * process_it - our "processing" work is kicked off here
 */
static void process_it(struct sed_ds *kd, struct sed_ds *kdret)
{
	//print_hex_dump_bytes("kd: ", DUMP_PREFIX_OFFSET, kd, sizeof(struct sed_ds));
	switch (kd->data_xform) {
	case XF_NONE:
		pr_debug("data transform type: XF_NONE\n");
		// nothing to do
		break;
	case XF_ENCRYPT:
		pr_debug("data transform type: XF_ENCRYPT\n");
		encrypt_decrypt_payload(WORK_IS_ENCRYPT, kd, kdret);
		break;
	case XF_DECRYPT:
		pr_debug("data transform type: XF_DECRYPT\n");
		encrypt_decrypt_payload(WORK_IS_DECRYPT, kd, kdret);
		break;
	}
}

/*
 * The key method - the ioctl - for our demo driver; note how we take into
 * account the fact that the ioctl's signtaure changed from 2.6.36 (as the
 * BKL was finally removed).
 *
 * CAREFUL!
 * What if two or more threads (or processes) concurrently invoke this driver
 * ioctl() method? There will be concurrency issues! ... which we haven't taken
 * care of here... This, and a lot more, will be discussed and detailed in this
 * book's last two chapters on Kernel Synchronization, a very real and important
 * topic.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static long ioctl_miscdrv(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int ioctl_miscdrv(struct inode *ino, struct file *filp, unsigned int cmd,
			 unsigned long arg)
#endif
{
	int ret = 0;
	struct sed_ds *kd, *kdret;
	struct stMyCtx *priv = gpriv;

	//pr_debug("In ioctl method, cmd=%d\n", _IOC_NR(cmd));

// TODO :: use dev_*() instead of pr_*()

	/* Verify stuff: is the ioctl's for us? etc.. */
	if (_IOC_TYPE(cmd) != IOCTL_LLKD_SED_MAGIC) {
		pr_warn("ioctl fail; magic # mismatch\n");
		return -ENOTTY;
	}
	if (_IOC_NR(cmd) > IOCTL_LLKD_SED_MAXIOCTL) {
		pr_warn("ioctl fail; invalid cmd?\n");
		return -ENOTTY;
	}

	ret = -ENOMEM;
	kd = kzalloc(sizeof(struct sed_ds), GFP_KERNEL);
	if (!kd)
		goto out_mem1;
	kdret = kzalloc(sizeof(struct sed_ds), GFP_KERNEL);
	if (!kdret)
		goto out_mem2;

	switch (cmd) {
	case IOCTL_LLKD_SED_IOC_ENCRYPT_MSG:
	case IOCTL_LLKD_SED_IOC_DECRYPT_MSG:
		pr_debug("In ioctl cmd option: %s\narg=0x%lx\n",
			 (cmd == IOCTL_LLKD_SED_IOC_ENCRYPT_MSG ? "encrypt" : "decrypt"), arg);
#if 0	// only allow root?
		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
#endif
		ret = -EFAULT;
		if (copy_from_user(kd, (struct sed_ds *)arg, sizeof(struct sed_ds))) {
			pr_warn("copy_from_user() failed\n");
			goto out_cftu;
		}
		pr_debug("xform=%d, len=%d\n", kd->data_xform, kd->len);
		print_hex_dump_bytes("payload: ", DUMP_PREFIX_OFFSET, kd->data, kd->len);
		process_it(kd, kdret);
		if (atomic_read(&priv->timed_out) == 1) {
			kdret->timed_out = 1;
			pr_debug("** timed out **\n");
		}
		print_hex_dump_bytes("ret payload: ", DUMP_PREFIX_OFFSET, kdret->data,
				     kdret->len);

		// write back processed payload to the user space process
		ret = -EFAULT;
		if (copy_to_user((struct sed_ds *)arg, (struct sed_ds *)kdret, sizeof(struct sed_ds))) {
			pr_warn("copy_to_user() failed\n");
			goto out_cftu;
		}
		break;
	default:
		kfree(kdret);
		kfree(kd);
		return -ENOTTY;
	}

	ret = 0;
out_cftu:
	kfree(kdret);
out_mem2:
	kfree(kd);
out_mem1:
	return ret;
}

/*----------- 'misc' methods -------------
 * dummy here...
 */
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

/*-------------- For registration with a kernel framework ---------------------*/
// Here, we shall register ourselves with the 'misc' kernel framework

/* The driver 'functionality' is encoded via the fops */
static const struct file_operations llkd_misc_fops = {
	.open = open_miscdrv,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	.unlocked_ioctl = ioctl_miscdrv,	// use the 'unlocked' version
#else
	.ioctl = ioctl_miscdrv,	// 'old' way
#endif
	.read = read_miscdrv,
	.write = write_miscdrv,
	.llseek = no_llseek,	// dummy, we don't support lseek(2)
	.release = close_miscdrv,
};

static struct miscdevice llkd_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,	/* kernel dynamically assigns a free minor# */
	.name = DRVNAME,	/* when misc_register() is invoked, the kernel
				 * will auto-create device file as /dev/llkd_miscdrv_rdwr;
				 *  also populated within /sys/class/misc/ and /sys/devices/virtual/misc/ */
	.mode = 0666,		/* ... dev node perms set as specified here */
	.fops = &llkd_misc_fops,	/* connect to this driver's 'functionality' */
};

static int __init sed1_drv_init(void)
{
	int ret = 0;
	struct device *dev;
	struct stMyCtx *priv = NULL;

	//--- Register with misc kernel framework
	ret = misc_register(&llkd_miscdev);
	if (ret) {
		pr_notice("misc device registration failed, aborting\n");
		return ret;
	}
	dev = llkd_miscdev.this_device;

	dev_info(dev, "LLKD %s misc driver (major # 10) registered, minor# = %d,\n"
		 "dev node is /dev/%s\n", DRVNAME, llkd_miscdev.minor, DRVNAME);

	/* We're using a global pointer to our private context structure. In a
	 * "real" driver, you'd typically hook up your private structure pointer
	 * to an appropriate member within the bus driver's structure(s);
	 * f.e., we could base this on a platform bus and do
	 *   plat0.dev.platform_data = priv;
	 * Here, for simplicity, we avoid attaching to a bus driver at all and
	 * simply employ the misc framework. This then implies that we access a
	 * global without explicit protection in various parts of this code; this
	 * issue is completely addressed in the next two chapters on kernel
	 * synchronization; here we simply ignore it.
	 */
	gpriv = priv = devm_kzalloc(dev, sizeof(struct stMyCtx), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	priv->dev = llkd_miscdev.this_device;
	atomic_set(&priv->timed_out, 0);

	// Initialize our kernel timer
	timer_setup(&priv->timr, timesup, 0);

	pr_info("init done (make_it_fail is %s)\n", make_it_fail == 1 ? "*on*" : "off");
	dev_dbg(dev, "loaded.\n");
	return ret;
}

static void __exit sed1_drv_exit(void)
{
	struct stMyCtx *priv = gpriv;

	dev_dbg(priv->dev, "unloading\n");
	del_timer_sync(&priv->timr);
	misc_deregister(&llkd_miscdev);
}

module_init(sed1_drv_init);
module_exit(sed1_drv_exit);
