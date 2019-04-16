/*
 * ch9/miscdrv/miscdrv.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch : Synchronization Primitives and How to Use Them
 ****************************************************************
 * Brief Description:
 *
 * For details, please refer the book, Ch 9.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
//#include <asm-generic/atomic-long.h>
#include "../../convenient.h"

#define OURMODNAME   "miscdrv"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKDC book:ch9/miscdrv: simple 'skeleton' misc char driver");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/*
 *  loff_t (llseek) (struct file *, loff_t, int);
        ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
        ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
int (*open) (struct inode *, struct file *);
        int (*release) (struct inode *, struct file *);
 */ 

static int open_miscdrv(struct inode *inode, struct file *filp)
{
	pr_info("%s:%s():\n"
		"wrt open file: f_flags = 0x%x, ref count = %ld\n",
	       OURMODNAME, __func__, filp->f_flags,
	       atomic_long_read(&filp->f_count));

	/* REQD:: XXX : spin_lock(inode->i_lock); .. then unlock 
	 *  do this for the CORRECT drv; miscdrv_enh.ko */
	pr_info("wrt the file's inode: "
		" uid=%u inode# = %lu size=%d\n",
	       OURMODNAME, __func__,
	       __kuid_val(inode->i_uid), inode->i_ino, inode->i_size);
	/* REQD:: XXX : spin_unlock(inode->i_lock); */

	PRINT_CTX();
	return 0;
}
static ssize_t read_miscdrv(struct file *filp, char __user *ubuf, size_t count, loff_t *off)
{
	PRINT_CTX();
	return count;
}

static const struct file_operations lkdc_misc_fops = {
	.open = open_miscdrv,
	.read = read_miscdrv,
//	.write = write_miscdrv,
//	.llseek = lseek_miscdrv,
//	.release = release_miscdrv,
};
static struct miscdevice lkdc_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "lkdc_miscdrv",
	.fops = &lkdc_misc_fops,
};

static int __init miscdrv_init(void)
{
	int ret;

	if ((ret = misc_register(&lkdc_miscdev))) {
		pr_notice("%s: misc device registration failed, aborting\n",
			       OURMODNAME);
		return ret;
	}
	pr_info("%s: Hi, LKDC misc driver (major # 10) registered, minor# = %d\n",
			OURMODNAME, lkdc_miscdev.minor);
	return 0;		/* success */
}

static void __exit miscdrv_exit(void)
{
	misc_deregister(&lkdc_miscdev);
	pr_debug("%s: LKDC misc driver deregistered, bye\n", OURMODNAME);
}

module_init(miscdrv_init);
module_exit(miscdrv_exit);
