/*
 * ioctl_llkd_kdrv.c
 *
 * A kernel module that demonstrates simple usage of the ioctl driver method
 * to interface with a userspace 'C' application (the ioctl_user_test.c).
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ioctl.h>

// copy_[to|from]_user()
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,11,0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

#include "ioctl_llkd.h"
#include "../../convenient.h"

#define OURMODNAME   "ioctl_llkd_kdrv"
MODULE_AUTHOR("<insert name here>");
MODULE_DESCRIPTION(
	"LLKD book:ch12/ioctl_intf: simple demo for using the ioctl interface");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int ioct_intf_major = 0,
	power = 1; /* 'powered on' by default */

/* 
 * The key method - the ioctl - for our demo driver; note how we take into
 * account the fact that the ioctl's signtaure changed from 2.6.36 (as the
 * BKL was finally removed).
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
static long ioct_intf_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int ioct_intf_ioctl(struct inode *ino, struct file *filp, unsigned int cmd,
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

#if 0
	/* Verify direction */
	if (_IOC_DIR(cmd) & _IOC_READ)
		/* userspace read => kernel-space -> userspace write operation */
		err =
		    !access_ok((void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		/* userspace write => userspace -> kernel-space read operation */
		err =
		    !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err)
		return -EFAULT;
#endif
	switch (cmd) {
	case IOCTL_LLKD_IOCRESET:
		MSG("In ioctl cmd option: IOCTL_LLKD_IOCRESET\n");
		break;
	case IOCTL_LLKD_IOCQPOWER:	/* Get: arg is pointer to result */
		MSG("In ioctl cmd option: IOCTL_LLKD_IOCQPOWER\n"
			"arg=0x%x (drv) power=%d\n", (unsigned int)arg, power);
		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
		retval = __put_user(power, (int __user *)arg);
		break;
	case IOCTL_LLKD_IOCSPOWER:	/* Set: arg is the value to set */
		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
		power = arg;
		MSG("In ioctl cmd option: IOCTL_LLKD_IOCSPOWER\n"
			"power=%d now.\n", power);
		break;
	default:
		return -ENOTTY;
	}
	return retval;
}

static struct file_operations ioct_intf_fops = {
	.llseek = no_llseek,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
	.unlocked_ioctl = ioct_intf_ioctl,	// use the 'unlocked' version!
#else
	.ioctl = ioct_intf_ioctl,
#endif
};

static int ioct_intf_open(struct inode *inode, struct file *filp)
{
	MSG("Device node with minor # %d being used\n", iminor(inode));

	switch (iminor(inode)) {
	case 0:
		filp->f_op = &ioct_intf_fops;
		break;
	default:
		return -ENXIO;
	}
	if (filp->f_op && filp->f_op->open)
		return filp->f_op->open(inode, filp);	/* Minor-specific open */

	return 0;
}

/* Major-wide open routine */
static struct file_operations ioct_intf_open_fops = {
	.open = ioct_intf_open,	/* just a means to get at the real open */
};

static int __init ioctl_llkd_kdrv_init(void)
{
	int result;

	MSG("ioct_intf_major=%d\n", ioct_intf_major);

	/*
	 * Register the major, and accept a dynamic number.
	 * The return value is the actual major # assigned.
	 */
	result = register_chrdev(ioct_intf_major, OURMODNAME, &ioct_intf_open_fops);
	if (result < 0) {
		MSG("register_chrdev() failed trying to get ioct_intf_major=%d\n",
		    ioct_intf_major);
		return result;
	}

	if (ioct_intf_major == 0)
		ioct_intf_major = result;	/* dynamic */
	MSG("registered:: ioct_intf_major=%d\n", ioct_intf_major);

	pr_info("%s initialized\n", OURMODNAME);
	return 0;		/* success */
}

static void ioctl_llkd_kdrv_cleanup(void)
{
	unregister_chrdev(ioct_intf_major, OURMODNAME);
	pr_info("%s removed\n", OURMODNAME);
}

module_init(ioctl_llkd_kdrv_init);
module_exit(ioctl_llkd_kdrv_cleanup);
