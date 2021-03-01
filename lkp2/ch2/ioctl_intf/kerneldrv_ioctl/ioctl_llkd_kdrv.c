/*
 * user_kernel_comm/ioctl_intf/kerneldrv_ioctl.c
 **************************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch : User-Kernel communication pathways
 **************************************************************************
 * Brief Description:
 *
 * For details, please refer the book.
 *
 * A kernel module that demonstrates simple usage of the ioctl driver method
 * to interface with a userspace 'C' application (the ioctl_user_test.c).
 * Architected as a simple device driver for a fictional 'device'; we use
 * the ioctl(2) system call to interface with the device from/to a usermode
 * 'C' application.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ioctl.h>

//--- copy_[to|from]_user()
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 11, 0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

#include "../ioctl_llkd.h"
#include "../../../convenient.h"

#define OURMODNAME   "ioctl_llkd_kdrv"
MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION(
"LLKD book:user_kernel_comm/ioctl_intf: simple demo for using the ioctl interface");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int ioctl_intf_major,
	power = 1; /* 'powered on' by default */

/*
 * The key method - the ioctl - for our demo driver; note how we take into
 * account the fact that the ioctl's signtaure changed from 2.6.36 (as the
 * BKL was finally removed).
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static long ioctl_intf_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int ioctl_intf_ioctl(struct inode *ino, struct file *filp, unsigned int cmd,
		     unsigned long arg)
#endif
{
	int retval = 0;

	pr_debug("In ioctl method, cmd=%d\n", _IOC_NR(cmd));

	/* Verify stuff: is the ioctl's for us? etc.. */
	if (_IOC_TYPE(cmd) != IOCTL_LLKD_MAGIC) {
		pr_info("ioctl fail; magic # mismatch\n");
		return -ENOTTY;
	}
	if (_IOC_NR(cmd) > IOCTL_LLKD_MAXIOCTL) {
		pr_info("ioctl fail; invalid cmd?\n");
		return -ENOTTY;
	}

	switch (cmd) {
	case IOCTL_LLKD_IOCRESET:
		pr_debug("In ioctl cmd option: IOCTL_LLKD_IOCRESET\n");
		/* ... Insert the code here to write to a control register to reset the
		 * device ... */
		break;
	case IOCTL_LLKD_IOCQPOWER:	/* Get: arg is pointer to result */
		pr_debug("In ioctl cmd option: IOCTL_LLKD_IOCQPOWER\n"
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
		pr_debug("In ioctl cmd option: IOCTL_LLKD_IOCSPOWER\n"
			"power=%d now.\n", power);
		break;
	default:
		return -ENOTTY;
	}
	return retval;
}

static const struct file_operations ioctl_intf_fops = {
	.llseek = no_llseek,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	.unlocked_ioctl = ioctl_intf_ioctl,	// use the 'unlocked' version
#else
	.ioctl = ioctl_intf_ioctl,   // 'old' way
#endif
};

static int ioctl_intf_open(struct inode *inode, struct file *filp)
{
	pr_debug("Device node with minor # %d being used\n", iminor(inode));

	switch (iminor(inode)) {
	case 0:
		filp->f_op = &ioctl_intf_fops;
		break;
	default:
		return -ENXIO;
	}
	if (filp->f_op && filp->f_op->open)
		return filp->f_op->open(inode, filp);	/* Minor-specific open */

	return 0;
}

/* Major-wide open routine */
static struct file_operations ioctl_intf_open_fops = {
	.open = ioctl_intf_open,	/* just a means to get at the real open */
};

static int __init ioctl_llkd_kdrv_init(void)
{
	int result;

	pr_debug("ioctl_intf_major=%d\n", ioctl_intf_major);

	/*
	 * Register the major, and accept a dynamic number.
	 * The return value is the actual major # assigned.
	 */
	result = register_chrdev(ioctl_intf_major, OURMODNAME, &ioctl_intf_open_fops);
	if (result < 0) {
		pr_info("register_chrdev() failed trying to get ioctl_intf_major=%d\n", ioctl_intf_major);
		return result;
	}

	if (ioctl_intf_major == 0)
		ioctl_intf_major = result;	/* dynamic */
	pr_debug("registered:: ioctl_intf_major=%d\n", ioctl_intf_major);

	pr_info("initialized\n");
	return 0;		/* success */
}

static void ioctl_llkd_kdrv_cleanup(void)
{
	unregister_chrdev(ioctl_intf_major, OURMODNAME);
	pr_info("removed\n");
}

module_init(ioctl_llkd_kdrv_init);
module_exit(ioctl_llkd_kdrv_cleanup);
