/*
 * ch9/miscdrv_rdwr/miscdrv_rdwr.c
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
#include <linux/mm.h>           // kvmalloc()
#include <linux/fs.h>		// the fops
#include <linux/uaccess.h>      // copy_to|from_user() macros
#include "../../convenient.h"

#define OURMODNAME   "miscdrv_rdwr"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKDC book:ch9/miscdrv_rdwr: simple 'skeleton' misc char driver with rdwr");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/*--- The driver 'methods' follow ---*/
/*
 * open_miscdrv_rdwr()
 * The driver's open 'method'; this 'hook' will get invoked by the kernel VFS
 * when the device file is opened. Here, we simply print out some relevant info.
 * The POSIX standard requires open() to return the file descriptor in success;
 * note, though, that this is done within the kernel VFS (when we return). So,
 * all we do here is return 0 indicating success.
 */
static int open_miscdrv_rdwr(struct inode *inode, struct file *filp)
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

	return 0;
}

/*
 * read_miscdrv_rdwr()
 * The driver's read 'method'; it has effectively 'taken over' the read syscall
 * functionality! Here, we simply print out some info.
 * The POSIX standard requires that the read() and write() system calls return
 * the number of bytes read or written on success, 0 on EOF and -1 (-ve errno)
 * on failure; we simply return 'count', pretending that we 'always succeed'.
 */
static ssize_t read_miscdrv_rdwr(struct file *filp, char __user *ubuf,
				size_t count, loff_t *off)
{
	int ret;
	void *kbuf = NULL;
	char kmsg[] = "Hey, from miscdrv_rdwr!";

	PRINT_CTX();
	pr_info("%s:%s():\n %s wants to read %ld bytes\n",
			OURMODNAME, __func__, current->comm, count);

	ret = -EINVAL;
	if (count < strlen(kmsg)) {
		pr_warn("%s:%s(): request # of bytes (%ld) is < message size (%ld), aborting read\n",
				OURMODNAME, __func__, count, strlen(kmsg));
		goto out_toolittle;
	}

	ret = -ENOMEM;
	kbuf = kvmalloc(count, GFP_KERNEL);
	if (unlikely(!kbuf)) {
		pr_warn("%s:%s(): kvmalloc() failed!\n", OURMODNAME, __func__);
		goto out_nomem;
	}

	/* In a 'real' driver, we would now actually read the content of the
	 * device hardware (or whatever) into the user supplied buffer 'ubuf'
	 * for 'count' bytes, and then copy it to the userspace process (via
	 * the copy_to_user() macro).
	 */
	ret = -EFAULT;
	if (copy_to_user(ubuf, kmsg, strlen(kmsg))) {
		pr_warn("%s:%s(): copy_to_user() failed\n", OURMODNAME, __func__);
		goto out_ctu;
	}
	ret = strlen(kmsg);

out_ctu:
	kvfree(kbuf);
out_nomem:
out_toolittle:
	return ret;
}

/*
 * write_miscdrv_rdwr()
 * The driver's write 'method'; it has effectively 'taken over' the write syscall
 * functionality! Here, we simply print out some info.
 * The POSIX standard requires that the read() and write() system calls return
 * the number of bytes read or written on success, 0 on EOF and -1 (-ve errno)
 * on failure; we simply return 'count', pretending that we 'always succeed'.
 */
static ssize_t write_miscdrv_rdwr(struct file *filp, const char __user *ubuf,
				size_t count, loff_t *off)
{
	int ret;
	void *kbuf = NULL;

	PRINT_CTX();
	pr_info("%s:%s():\n %s wants to write %ld bytes\n",
			OURMODNAME, __func__, current->comm, count);

	ret = -ENOMEM;
	kbuf = kvmalloc(count, GFP_KERNEL);
	if (unlikely(!kbuf)) {
		pr_warn("%s:%s(): kvmalloc() failed!\n", OURMODNAME, __func__);
		goto out_nomem;
	}

	/* Copy in the user supplied buffer 'ubuf' - the data content to write -
	 * via the copy_from_user() macro
	 */
	ret = -EFAULT;
	if (copy_from_user(kbuf, ubuf, count)) {
		pr_warn("%s:%s(): copy_from_user() failed\n", OURMODNAME, __func__);
		goto out_cfu;
	}

	/* In a 'real' driver, we would now actually write (for 'count' bytes)
	 * the content of the 'ubuf' buffer to the device hardware (or whatever),
	 * and then return.
	 */
	ret = count;

out_cfu:
	kvfree(kbuf);
out_nomem:
	return ret;
}

/*
 * close_miscdrv_rdwr()
 * The driver's close 'method'; this 'hook' will get invoked by the kernel VFS
 * when the device file is closed (technically, when the file ref count drops
 * to 0). Here, we simply print out some info, and return 0 indicating success.
 */
static int close_miscdrv_rdwr(struct inode *inode, struct file *filp)
{
        PRINT_CTX(); // displays process (or intr) context info

        /* REQD:: XXX : spin_lock(filp->f_lock); .. then unlock 
         *  do this for the CORRECT drv; miscdrv_enh.ko */
        pr_info("%s:%s(): filename: \"%s\"\n",
               OURMODNAME, __func__, filp->f_path.dentry->d_iname);
        /* REQD:: XXX : spin_unlock(inode->f_lock); */
        return 0;
}

/* The driver 'functionality' is encoded via the fops */
static const struct file_operations lkdc_misc_fops = {
	.open = open_miscdrv_rdwr,
	.read = read_miscdrv_rdwr,
	.write = write_miscdrv_rdwr,
	.llseek = no_llseek,             // dummy, we don't support lseek(2)
	.release = close_miscdrv_rdwr,
};

static struct miscdevice lkdc_miscdev = {
	.minor = MISC_DYNAMIC_MINOR, // kernel dynamically assigns a free minor#
	.name = "lkdc_miscdrv_rdwr",
	    // populated within /sys/class/misc/ and /sys/devices/virtual/misc/
	.fops = &lkdc_misc_fops,     // connect to 'functionality'
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
