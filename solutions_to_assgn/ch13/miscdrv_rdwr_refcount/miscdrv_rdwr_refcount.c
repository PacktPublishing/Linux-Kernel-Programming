/*
 * solutions_to_assgn/ch7/miscdrv_rdwr_refcount/miscdrv_rdwr_refcount.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming-Part-2
 *
 * From: Ch 7 : Kernel Synchronization - Part 2
 ****************************************************************
 * Question:
 *  Convert our earlier ch6/2_miscdrv_rdwr_spinlock/miscdrv_rdwr_spinlock.c
 * driver code; it has the integers ga and gb , which, when being read or
 * written were protected via a spinlock. Now make them refcount variables and
 * use the appropriate refcount_t APIs when working on them. (Careful! don't
 * allow their values to go out of the allowed range [0..INT_MAX] !)
 *
 * For details, please refer the book, Ch 7.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>         // k[m|z]alloc(), k[z]free(), ...
#include <linux/mm.h>           // kvmalloc()
#include <linux/fs.h>		// the fops structure

// copy_[to|from]_user()
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 11, 0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/refcount.h>
#include "../../convenient.h"

#define OURMODNAME   "miscdrv_rdwr_refcount"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION(
"solutions_to_assgn/ch7/miscdrv_rdwr_refcount: simple misc driver rewritten to use refcount_t interfaces");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static refcount_t ga = REFCOUNT_INIT(5); /* ga will be init to 5 */
static refcount_t gb = REFCOUNT_INIT(5); /* gb will be init to 5 */

/* The driver 'context' data structure;
 * all relevant 'state info' reg the driver is here.
 */
struct drv_ctx {
	struct device *dev;
	int tx, rx, err, myword;
	u32 config1, config2;
	u64 config3;
#define MAXBYTES    128
	char oursecret[MAXBYTES];
	struct mutex mutex;  // this mutex protects this data structure
	spinlock_t spinlock; // ...so does this spinlock
};
static struct drv_ctx *ctx;

static inline void display_stats(int show_stats)
{
	struct device *dev = ctx->dev;

	if (1 == show_stats) {
		spin_lock(&ctx->spinlock);
		dev_info(dev, "stats: tx=%d, rx=%d\n", ctx->tx, ctx->rx);
		spin_unlock(&ctx->spinlock);
	}
}

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
	struct device *dev = ctx->dev;

	PRINT_CTX();		// displays process (or intr) context info

	refcount_inc(&ga);
	refcount_dec(&gb);

	dev_info(dev, " filename: \"%s\"\n"
		" wrt open file: f_flags = 0x%x\n"
		" ga = %d, gb = %d\n", filp->f_path.dentry->d_iname, filp->f_flags,
		refcount_read(&ga), refcount_read(&gb));

	display_stats(1);
	return 0;
}

/*
 * read_miscdrv_rdwr()
 * The driver's read 'method'; it has effectively 'taken over' the read syscall
 * functionality!
 * The POSIX standard requires that the read() and write() system calls return
 * the number of bytes read or written on success, 0 on EOF and -1 (-ve errno)
 * on failure; here, we copy the 'secret' from our driver context structure
 * to the userspace app.
 */
static ssize_t read_miscdrv_rdwr(struct file *filp, char __user *ubuf,
				size_t count, loff_t *off)
{
	int ret = count, secret_len, err_path = 0;
	struct device *dev = ctx->dev;

	spin_lock(&ctx->spinlock);
	secret_len = strlen(ctx->oursecret);
	spin_unlock(&ctx->spinlock);

	PRINT_CTX();
	dev_info(dev, "%s wants to read (upto) %zu bytes\n", current->comm, count);

	ret = -EINVAL;
	if (count < MAXBYTES) {
		dev_warn(dev, "request # of bytes (%zu) is < required size"
			" (%d), aborting read\n", count, MAXBYTES);
		err_path = 1;
		goto out_notok;
	}
	if (secret_len <= 0) {
		dev_warn(dev, "whoops, something's wrong, the 'secret' isn't"
			" available..; aborting read\n");
		err_path = 1;
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
	mutex_lock(&ctx->mutex);
	/* Why don't we just use the spinlock??
	 * Because - VERY IMP! - remember that the spinlock can only be used when
	 * the critical section will not sleep or block in any manner; here,
	 * the critical section invokes the copy_to_user(); it very much can
	 * cause a 'sleep' (a schedule()) to occur.
	 */
	if (copy_to_user(ubuf, ctx->oursecret, secret_len)) {
		dev_warn(dev, "copy_to_user() failed\n");
		err_path = 1;
		goto out_ctu;
	}
	ret = secret_len;

	// Update stats
	ctx->tx += secret_len; // our 'transmit' is wrt this driver
	dev_info(dev, " %d bytes read, returning... (stats: tx=%d, rx=%d)\n",
			secret_len, ctx->tx, ctx->rx);
out_ctu:
	mutex_unlock(&ctx->mutex);
	display_stats(err_path);
out_notok:
	return ret;
}

/*
 * write_miscdrv_rdwr()
 * The driver's write 'method'; it has effectively 'taken over' the write syscall
 * functionality!
 * The POSIX standard requires that the read() and write() system calls return
 * the number of bytes read or written on success, 0 on EOF and -1 (-ve errno)
 * on failure; Here, we accept the string passed to us and update our 'secret'
 * value to it.
 */
static ssize_t write_miscdrv_rdwr(struct file *filp, const char __user *ubuf,
				size_t count, loff_t *off)
{
	int ret, err_path = 0;
	void *kbuf = NULL;
	struct device *dev = ctx->dev;

	PRINT_CTX();
	dev_info(dev, "%s wants to write %zu bytes\n", current->comm, count);

	ret = -ENOMEM;
	kbuf = kvmalloc(count, GFP_KERNEL);
	if (unlikely(!kbuf)) {
		dev_warn(dev, "kvmalloc() failed!\n");
		err_path = 1;
		goto out_nomem;
	}
	memset(kbuf, 0, count);

	/* Copy in the user supplied buffer 'ubuf' - the data content to write -
	 * via the copy_from_user() macro.
	 * (FYI, the copy_from_user() macro is the *right* way to copy data from
	 * kernel-space to userspace; the parameters are:
	 *  'to-buffer', 'from-buffer', count
	 *  Returns 0 on success, i.e., non-zero return implies an I/O fault).
	 */
	ret = -EFAULT;
	if (copy_from_user(kbuf, ubuf, count)) {
		dev_warn(dev, "copy_from_user() failed\n");
		err_path = 1;
		goto out_cfu;
	}

	/* In a 'real' driver, we would now actually write (for 'count' bytes)
	 * the content of the 'ubuf' buffer to the device hardware (or whatever),
	 * and then return.
	 * Here, we first acquire the spinlock, then write the just-accepted
	 * new 'secret' into our driver 'context' structure, and unlock.
	 */
	spin_lock(&ctx->spinlock);
	strlcpy(ctx->oursecret, kbuf, (count > MAXBYTES ? MAXBYTES : count));
#if 0
	print_hex_dump_bytes("ctx ", DUMP_PREFIX_OFFSET,
				ctx, sizeof(struct drv_ctx));
#endif
	// Update stats
	ctx->rx += count;	// our 'receive' is wrt userspace

	ret = count;
	dev_info(dev, " %zu bytes written, returning... (stats: tx=%d, rx=%d)\n",
		count, ctx->tx, ctx->rx);

	spin_unlock(&ctx->spinlock);
out_cfu:
	kvfree(kbuf);
	display_stats(err_path);
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

	PRINT_CTX();		// displays process (or intr) context info

	refcount_dec(&ga);
	refcount_inc(&gb);

	dev_info(dev, "filename: \"%s\"\n ga = %d, gb = %d\n",
		filp->f_path.dentry->d_iname,
		refcount_read(&ga), refcount_read(&gb));
	display_stats(1);

	return 0;
}

/* The driver 'functionality' is encoded via the fops */
static const struct file_operations llkd_misc_fops = {
	.open = open_miscdrv_rdwr,
	.read = read_miscdrv_rdwr,
	.write = write_miscdrv_rdwr,
	.llseek = no_llseek,    // dummy, we don't support lseek(2)
	.release = close_miscdrv_rdwr,
	/* As you learn more reg device drivers, you'll realize that the
	 * ioctl() would be a very useful method here. As an exercise,
	 * implement an ioctl method; when issued with the 'GETSTATS' 'command',
	 * it should return the statistics (tx, rx, errors) to the calling app
	 */
};

static struct miscdevice llkd_miscdev = {
	.minor = MISC_DYNAMIC_MINOR, // kernel dynamically assigns a free minor#
	.name = "llkd_miscdrv_rdwr_refcount",
	    // populated within /sys/class/misc/ and /sys/devices/virtual/misc/
	.mode = 0666,       /* ... dev node perms set as specified here */
	.fops = &llkd_misc_fops,     // connect to 'functionality'
};

static int __init miscdrv_init_refcount(void)
{
	int ret;

	ret = misc_register(&llkd_miscdev);
	if (ret < 0) {
		pr_notice("misc device registration failed, aborting\n");
		return ret;
	}
	pr_info("LLKD misc driver (major # 10) registered, minor# = %d,"
		" dev node is %s\n", llkd_miscdev.minor, llkd_miscdev.name);

	/*
     * A 'managed' kzalloc(): use the 'devres' API devm_kzalloc() for mem
     * alloc; why? as the underlying kernel devres framework will take care of
     * freeing the memory automatically upon driver 'detach' or when the driver
     * is unloaded from memory
     */
	ctx = kzalloc(sizeof(struct drv_ctx), GFP_KERNEL);
	if (unlikely(!ctx))
		return -ENOMEM;

	mutex_init(&ctx->mutex);
	spin_lock_init(&ctx->spinlock);

	/* Retrieve the device pointer for this device */
	ctx->dev = llkd_miscdev.this_device;

	strlcpy(ctx->oursecret, "initmsg", 8);
		/* Why don't we protect the above strlcpy() with the mutex / spinlock?
		 * It's working on shared writable data, yes?
		 * No; this is the init code; it's guaranteed to run in exactly
		 * one context (typically the insmod(8) process), thus there is
		 * no concurrency possible here. The same goes for the cleanup
		 * code path.
		 */

	dev_dbg(ctx->dev, "A sample print via the dev_dbg(): driver initialized\n");
	return 0;		/* success */
}

static void __exit miscdrv_exit_refcount(void)
{
	mutex_destroy(&ctx->mutex);
	misc_deregister(&llkd_miscdev);
	pr_info("%s: LLKD misc driver deregistered, bye\n", OURMODNAME);
}

module_init(miscdrv_init_refcount);
module_exit(miscdrv_exit_refcount);
