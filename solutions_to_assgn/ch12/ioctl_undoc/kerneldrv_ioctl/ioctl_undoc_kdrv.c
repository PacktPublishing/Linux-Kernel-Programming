/*
 * solutions_to_assgn/ch12/ioctl_undoc/kerneldrv_ioctl/ioctl_undoc_kdrv.c
 **************************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 12 : User - kernel communication pathways
 **************************************************************************
 * Assignment : ch12: ioctl #2: 'ioctl_undoc':
 *
 * Using the provided ch12/ioctl_intf/ code as a template, write a userspace
 * 'C' application and a kernel-space (char) device driver implementing the
 * ioctl method. Add in a 'driver context' data structure (that we use in
 * several examples), allocate and initialize it. Now, in addition to the
 * earlier three ioctl 'commands' we use, setup a fourth 'undocumented' command
 * (you can call it IOCTL_LLKD_IOCQDRVSTAT). It's job: when queried from
 * userspace via ioctl(2), it must return the contents of the 'driver context'
 * data structure to userspace; the userspace 'C' app must print out the
 * content.
 *
 * For details refer the book, Ch 12.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/slab.h>

// copy_[to|from]_user()
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,11,0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

#include "../ioctl_llkd.h"
#include "../../../convenient.h"

#define OURMODNAME   "ioctl_undoc_kdrv"
MODULE_AUTHOR("<insert name here>");
MODULE_DESCRIPTION(
	"LLKD book:solutions_to_assgn/ch12/ioctl_undoc: a soln to ch12 assignment #1");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int ioctl_intf_major = 0,
	power = 1; /* 'powered on' by default */

/* 
 * The key method - the ioctl - for our demo driver; note how we take into
 * account the fact that the ioctl's signtaure changed from 2.6.36 (as the
 * BKL was finally removed).
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
static long ioctl_intf_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int ioctl_intf_ioctl(struct inode *ino, struct file *filp, unsigned int cmd,
		     unsigned long arg)
#endif
{
	int retval = 0;

	MSG("In ioctl method, cmd=%d\n", _IOC_NR(cmd));

	/* Verify stuff: is the ioctl's for us? etc.. */
	if (_IOC_TYPE(cmd) != IOCTL_LLKD_MAGIC) {
		MSG("ioctl fail; magic # mismatch\n");
		return -ENOTTY;
	}
	if (_IOC_NR(cmd) > IOCTL_LLKD_MAXIOCTL) {
		MSG("ioctl fail; invalid cmd?\n");
		return -ENOTTY;
	}

	switch (cmd) {
	case IOCTL_LLKD_IOCRESET:
		MSG("In ioctl cmd option: IOCTL_LLKD_IOCRESET\n");
		/* ... Insert the code here to write to a control register to reset the
		 * device ... */
		break;
	case IOCTL_LLKD_IOCQPOWER:	/* Get: arg is pointer to result */
		MSG("In ioctl cmd option: IOCTL_LLKD_IOCQPOWER\n"
			"arg=0x%x (drv) power=%d\n", (unsigned int)arg, power);
		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
		/* ... Insert the code here to read a status register to query the
		 * power state of the device ...
		 * here, imagine we've done that and placed it into a variable 'power'
		 */
		retval = __put_user(power, (int __user *)arg);
		break;
	case IOCTL_LLKD_IOCSPOWER:	/* Set: arg is the value to set */
		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
		power = arg;
		/* ... Insert the code here to write a control register to set the
		 * power state of the device ...
		 */
		MSG("In ioctl cmd option: IOCTL_LLKD_IOCSPOWER\n"
			"power=%d now.\n", power);
		break;
	case IOCTL_LLKD_IOCQDRVSTAT:	/* the new 'undocumented' ioctl ! */
		pr_info("%s:%s(): new undoc ioctl cmd!\n", OURMODNAME, __func__);
		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
		break;
	default:
		return -ENOTTY;
	}
	return retval;
}

static int ioctl_intf_minor_open(struct inode *ino, struct file *filp)
{
	struct drv_ctx *drvctx = NULL;

	drvctx = kzalloc(sizeof(struct drv_ctx), GFP_KERNEL);
	if (!drvctx) {
		pr_warn("%s: kmalloc failed\n", OURMODNAME);
		return -ENOMEM;
	}
	drvctx->power = 1;

	/* Useful: we can now gain access to the drv context anywhere! */
	filp->private_data = drvctx;

	MSG("allocated and init the driver context structure\n");
	return 0;
}

static int ioctl_intf_minor_close(struct inode *ino, struct file *filp)
{
	kfree(filp->private_data);
	MSG("freed the driver context structure\n");
	return 0;
}


static struct file_operations ioctl_intf_fops = {
	.open = ioctl_intf_minor_open,
	.llseek = no_llseek,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
	.unlocked_ioctl = ioctl_intf_ioctl,	// use the 'unlocked' version
#else
	.ioctl = ioctl_intf_ioctl,   // 'old' way
#endif
	.release = ioctl_intf_minor_close,
};

static int __init ioctl_undoc_kdrv_init(void)
{
	int result;

	MSG("ioctl_intf_major=%d\n", ioctl_intf_major);

	/*
	 * Register the major, and accept a dynamic number.
	 * The return value is the actual major # assigned.
	 */
	result = register_chrdev(ioctl_intf_major, OURMODNAME, &ioctl_intf_fops);
	if (result < 0) {
		MSG("register_chrdev() failed trying to get ioctl_intf_major=%d\n",
		    ioctl_intf_major);
		return result;
	}

	if (ioctl_intf_major == 0)
		ioctl_intf_major = result;	/* dynamic */
	MSG("registered:: ioctl_intf_major=%d\n", ioctl_intf_major);

	pr_info("%s initialized\n", OURMODNAME);
	return 0;		/* success */
}

static void ioctl_undoc_kdrv_cleanup(void)
{
	unregister_chrdev(ioctl_intf_major, OURMODNAME);
	pr_info("%s removed\n", OURMODNAME);
}

module_init(ioctl_undoc_kdrv_init);
module_exit(ioctl_undoc_kdrv_cleanup);
