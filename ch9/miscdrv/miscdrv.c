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
#include "../../convenient.h"

#define OURMODNAME   "miscdrv"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKDC book:ch9/miscdrv: simple 'skeleton' misc char driver");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/*
 * open_miscdrv()
 * The driver's open 'method'; this 'hook' will get invoked by the kernel VFS
 * when the device file is opened. Here, we simply print out some relevant info.
 * The POSIX standard requires open() to return the file descriptor in success;
 * note, though, that this is done within the kernel VFS (when we return). So,
 * all we do here is return 0 indicating success.
 */
static int open_miscdrv(struct inode *inode, struct file *filp)
{
	PRINT_CTX(); // displays process (or intr) context info

	/* REQD:: XXX : spin_lock(filp->f_lock); .. then unlock 
	 *  do this for the CORRECT drv; miscdrv_enh.ko */
	pr_info("%s:%s():\n"
		" filename: \"%s\"\n"
		" wrt open file: f_flags = 0x%x, ref count = %ld\n",
	       OURMODNAME, __func__, filp->f_path.dentry->d_iname,
	       filp->f_flags, atomic_long_read(&filp->f_count));
	/* REQD:: XXX : spin_unlock(inode->f_lock); */

#if 0
	/* REQD:: XXX : spin_lock(inode->i_lock); .. then unlock 
	 *  do this for the CORRECT drv; miscdrv_enh.ko */
	pr_info("wrt the file's inode: "
		" uid=%u inode# = %lu size=%lld\n",
	       OURMODNAME, __func__,
	       __kuid_val(inode->i_uid), inode->i_ino, inode->i_size);
	/* REQD:: XXX : spin_unlock(inode->i_lock); */
#endif
	return 0;
}

/*
 * read_miscdrv()
 * The driver's read 'method'; it has effectively 'taken over' the read syscall
 * functionality! Here, we simply print out some info.
 * The POSIX standard requires that the read() and write() system calls return
 * the number of bytes read or written on success, 0 on EOF and -1 (-ve errno)
 * on failure; we simply return 'count', pretending that we 'always succeed'.
 */
static ssize_t read_miscdrv(struct file *filp, char __user *ubuf,
				size_t count, loff_t *off)
{
	pr_info("%s:%s():\n", OURMODNAME, __func__);
	return count;
}

/*
 * write_miscdrv()
 * The driver's write 'method'; it has effectively 'taken over' the write syscall
 * functionality! Here, we simply print out some info.
 * The POSIX standard requires that the read() and write() system calls return
 * the number of bytes read or written on success, 0 on EOF and -1 (-ve errno)
 * on failure; we simply return 'count', pretending that we 'always succeed'.
 */
static ssize_t write_miscdrv(struct file *filp, const char __user *ubuf,
				size_t count, loff_t *off)
{
	pr_info("%s:%s():\n", OURMODNAME, __func__);
	return count;
}

/*
 * close_miscdrv()
 * The driver's close 'method'; this 'hook' will get invoked by the kernel VFS
 * when the device file is closed (technically, when the file ref count drops
 * to 0). Here, we simply print out some relevant info.
 * We simply return 0 indicating success.
 */
static int close_miscdrv(struct inode *inode, struct file *filp)
{
	/* REQD:: XXX : spin_lock(filp->f_lock); .. then unlock 
	 *  do this for the CORRECT drv; miscdrv_enh.ko */
	pr_info("%s:%s(): filename: \"%s\"\n",
	       OURMODNAME, __func__, filp->f_path.dentry->d_iname);
	/* REQD:: XXX : spin_unlock(inode->f_lock); */
	return 0;
}

static const struct file_operations lkdc_misc_fops = {
	.open = open_miscdrv,
	.read = read_miscdrv,
	.write = write_miscdrv,
	.release = close_miscdrv,
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
	pr_info("%s: LKDC misc driver (major # 10) registered, minor# = %d\n",
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
