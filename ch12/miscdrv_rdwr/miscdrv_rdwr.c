/*
 * ch12/miscdrv_rdwr/miscdrv_rdwr.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 12 : Writing a Simple Misc Character Device Driver
 ****************************************************************
 * Brief Description:
 * This driver is built upon our previous 'skeleton' ../miscdrv/ misc
 * framework driver. The key difference: we use a few global data items within
 * a driver 'private' data structure throughout.
 * On init, we allocate memory to it and initialize it; one of the members
 * within is a so-called secret (the 'oursecret' member along with some fake
 * statistics and config words).
 * Importantly here, we perform (basic) I/O - reading and writing:
 * when a userpace process (or thread) opens our device file and issues a
 * read(2) upon it, we pass back the 'secret' data string to it.
 * When a user mode process writes data to us, we consider that data to be the
 * new 'secret' string and update it here (in driver memory).
 *
 * For details, please refer the book, Ch 12.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>		// k[m|z]alloc(), k[z]free(), ...
#include <linux/mm.h>		// kvmalloc()
#include <linux/fs.h>		// the fops
#include <linux/sched.h>	// get_task_comm()

// copy_[to|from]_user()
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 11, 0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

#include "../../convenient.h"

#define OURMODNAME   "miscdrv_rdwr"
MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LLKD book:ch12/miscdrv_rdwr: simple misc char driver with"
" a 'secret' to read/write");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int ga, gb = 1;		/* ignore for now ... */

/*
 * The driver 'context' (or private) data structure;
 * all relevant 'state info' regarding the driver is here.
 */
struct drv_ctx {
	struct device *dev;
	int tx, rx, err, myword;
	u32 config1, config2;
	u64 config3;
#define MAXBYTES    128		/* Must match the userspace app; we should actually
				 * use a common header file for things like this */
	char oursecret[MAXBYTES];
};
static struct drv_ctx *ctx;

/*--- The driver 'methods' follow ---*/
/*
 * open_miscdrv_rdwr()
 * The driver's open 'method'; this 'hook' will get invoked by the kernel VFS
 * when the device file is opened. Here, we simply print out some relevant info.
 * The POSIX standard requires open() to return the file descriptor in success;
 * note, though, that this is done within the kernel VFS (when we return). So,
 * all we do here is return 0 indicating success.
 * (The nonseekable_open(), in conjunction with the fop's llseek pointer set to
 * no_llseek, tells the kernel that our device is not seek-able).
 */
static int open_miscdrv_rdwr(struct inode *inode, struct file *filp)
{
	struct device *dev = ctx->dev;
	char *buf = kzalloc(PATH_MAX, GFP_KERNEL);

	if (unlikely(!buf))
		return -ENOMEM;

	PRINT_CTX();	// displays process (or atomic) context info
	ga++;
	gb--;
	dev_info(dev, " opening \"%s\" now; wrt open file: f_flags = 0x%x\n",
		file_path(filp, buf, PATH_MAX), filp->f_flags);
	kfree(buf);

	return nonseekable_open(inode, filp);
}

/*
 * read_miscdrv_rdwr()
 * The driver's read 'method'; it has effectively 'taken over' the read syscall
 * functionality!
 * The POSIX standard requires that the read() and write() system calls return
 * the number of bytes read or written on success, 0 on EOF (for read), and -1
 * (-ve errno) on failure; here, we copy the 'secret' from our driver context
 * structure to the userspace app.
 */
static ssize_t read_miscdrv_rdwr(struct file *filp, char __user *ubuf,
				 size_t count, loff_t *off)
{
	int ret = count, secret_len = strnlen(ctx->oursecret, MAXBYTES);
	struct device *dev = ctx->dev;
	char tasknm[TASK_COMM_LEN];

	PRINT_CTX();
	dev_info(dev, "%s wants to read (upto) %zu bytes\n", get_task_comm(tasknm, current), count);

	ret = -EINVAL;
	if (count < MAXBYTES) {
		dev_warn(dev, "request # of bytes (%zu) is < required size"
			" (%d), aborting read\n", count, MAXBYTES);
		goto out_notok;
	}
	if (secret_len <= 0) {
		dev_warn(dev, "whoops, something's wrong, the 'secret' isn't"
			" available..; aborting read\n");
		goto out_notok;
	}

	/* In a 'real' driver, we would now actually read the content of the
	 * device hardware (or whatever) into the user supplied buffer 'ubuf'
	 * for 'count' bytes, and then copy it to the userspace process (via
	 * the copy_to_user() routine).
	 * (FYI, the copy_to_user() routine is the *right* way to copy data from
	 * userspace to kernel-space; the parameters are:
	 *  'to-buffer' (dest), 'from-buffer' (src), count (# of bytes)
	 * Returns 0 on success, i.e., non-zero return implies an I/O fault).
	 * Here, we simply copy the content of our context structure's 'secret'
	 * member to userspace.
	 */
	ret = -EFAULT;
	if (copy_to_user(ubuf, ctx->oursecret, secret_len)) {
		dev_warn(dev, "copy_to_user() failed\n");
		goto out_notok;
	}
	ret = secret_len;

	// Update stats
	ctx->tx += secret_len;	// our 'transmit' is wrt this driver
	dev_info(dev, " %d bytes read, returning... (stats: tx=%d, rx=%d)\n",
		secret_len, ctx->tx, ctx->rx);
 out_notok:
	return ret;
}

/*
 * write_miscdrv_rdwr()
 * The driver's write 'method'; it has effectively 'taken over' the write syscall
 * functionality!
 * The POSIX standard requires that the read() and write() system calls return
 * the number of bytes read or written on success, 0 on EOF (for read), and -1
 * (-ve errno) on failure; here, we copy the 'secret' from our driver context
 * structure to the userspace app.
 */
static ssize_t write_miscdrv_rdwr(struct file *filp, const char __user *ubuf,
				  size_t count, loff_t *off)
{
	int ret = count;
	void *kbuf = NULL;
	struct device *dev = ctx->dev;
	char tasknm[TASK_COMM_LEN];

	PRINT_CTX();
	if (unlikely(count > MAXBYTES)) {	/* paranoia */
		dev_warn(dev, "count %zu exceeds max # of bytes allowed, "
			"aborting write\n", count);
		goto out_nomem;
	}
	dev_info(dev, "%s wants to write %zu bytes\n", get_task_comm(tasknm, current), count);

	ret = -ENOMEM;
	kbuf = kvmalloc(count, GFP_KERNEL);
	if (unlikely(!kbuf))
		goto out_nomem;
	memset(kbuf, 0, count);

	/* Copy in the user supplied buffer 'ubuf' - the data content to write -
	 * via the copy_from_user() macro.
	 * (FYI, the copy_from_user() macro is the *right* way to copy data from
	 * userspace to kernel-space; the parameters are:
	 *  'to-buffer', 'from-buffer', count
	 *  Returns 0 on success, i.e., non-zero return implies an I/O fault).
	 */
	ret = -EFAULT;
	if (copy_from_user(kbuf, ubuf, count)) {
		dev_warn(dev, "copy_from_user() failed\n");
		goto out_cfu;
	}

	/* In a 'real' driver, we would now actually write (for 'count' bytes)
	 * the content of the 'ubuf' buffer to the device hardware (or whatever),
	 * and then return.
	 * Here, we do nothing, we just pretend we've done everything :-)
	 */
	strlcpy(ctx->oursecret, kbuf, (count > MAXBYTES ? MAXBYTES : count));
#if 0
	/* Might be useful to actually see a hex dump of the driver 'context' */
	print_hex_dump_bytes("ctx ", DUMP_PREFIX_OFFSET,
			     ctx, sizeof(struct drv_ctx));
#endif
	// Update stats
	ctx->rx += count;	// our 'receive' is wrt userspace

	ret = count;
	dev_info(dev, " %zu bytes written, returning... (stats: tx=%d, rx=%d)\n",
		count, ctx->tx, ctx->rx);
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
	struct device *dev = ctx->dev;
	char *buf = kzalloc(PATH_MAX, GFP_KERNEL);

	if (unlikely(!buf))
		return -ENOMEM;

	PRINT_CTX();		// displays process (or intr) context info
	ga--;
	gb++;
	dev_info(dev, " filename: \"%s\"\n", file_path(filp, buf, PATH_MAX));
	kfree(buf);

	return 0;
}

/* The driver 'functionality' is encoded via the fops */
static const struct file_operations llkd_misc_fops = {
	.open = open_miscdrv_rdwr,
	.read = read_miscdrv_rdwr,
	.write = write_miscdrv_rdwr,
	.llseek = no_llseek,	// dummy, we don't support lseek(2)
	.release = close_miscdrv_rdwr,
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
	.name = "llkd_miscdrv_rdwr",	/* when misc_register() is invoked, the kernel
		 * will auto-create device file as /dev/llkd_miscdrv_rdwr;
		 *  also populated within /sys/class/misc/ and /sys/devices/virtual/misc/ */
	.mode = 0666,		/* ... dev node perms set as specified here */
	.fops = &llkd_misc_fops,	/* connect to this driver's 'functionality' */
};

static int __init miscdrv_rdwr_init(void)
{
	int ret = 0;
	struct device *dev;

	ret = misc_register(&llkd_miscdev);
	if (ret) {
		pr_notice("%s: misc device registration failed, aborting\n", OURMODNAME);
		return ret;
	}
	/* Retrieve the device pointer for this device */
	dev = llkd_miscdev.this_device;

	pr_info("LLKD misc driver (major # 10) registered, minor# = %d,"
		" dev node is /dev/%s\n", llkd_miscdev.minor, llkd_miscdev.name);

	/*
	 * A 'managed' kzalloc(): use the 'devres' API devm_kzalloc() for mem
	 * alloc; why? as the underlying kernel devres framework will take care of
	 * freeing the memory automatically upon driver 'detach' or when the driver
	 * is unloaded from memory
	 */
	ctx = devm_kzalloc(dev, sizeof(struct drv_ctx), GFP_KERNEL);
	if (unlikely(!ctx))
		return -ENOMEM;

	ctx->dev = dev;
	/* Initialize the "secret" value :-) */
	strlcpy(ctx->oursecret, "initmsg", 8);
	dev_dbg(ctx->dev, "A sample print via the dev_dbg(): driver initialized\n");

	return 0;		/* success */
}

static void __exit miscdrv_rdwr_exit(void)
{
	misc_deregister(&llkd_miscdev);
	pr_info("LLKD misc (rdwr) driver deregistered, bye\n");
}

module_init(miscdrv_rdwr_init);
module_exit(miscdrv_rdwr_exit);
