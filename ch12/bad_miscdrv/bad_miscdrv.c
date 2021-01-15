/*
 * ch12/bad_miscdrv_rdwr/bad_miscdrv_rdwr.c
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
 * This driver is built upon our previous ../miscdrv_rdwr/ misc driver.
 * The key difference: we deliberately insert buggy code - selectable
 * via defines in the source below - that make the read and/or write methods
 * buggy.
 * The read bug merely results in a 'bad' read reported by the kernel; the
 * write bug, when enabled, is in fact *a dangerous for security* one: it modifies
 * the current process context's task structure's 'cred->uid' member to zero,
 * thus making the process gain root access - in effect, a 'privesc'
 * (privilege escalation)!
 *
 * Do note: this security 'hack' is a very deliberately contrived and not a
 * particularly clever one; we certainly do not expect driver authors to do
 * stuff like this - writing into the process task structure! Really, the
 * reason it's demonstrated here is to show precisely why thinking about
 * security and avoiding sloppy coding practices is crucial.
 * Also, it *only* works if the device file permissions allow a write for
 * public non-root users!
 *
 * For details, please refer the book, Ch 12.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>         // k[m|z]alloc(), k[z]free(), ...
#include <linux/mm.h>           // kvmalloc()
#include <linux/fs.h>		// the fops

// copy_[to|from]_user()
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 11, 0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

#include <linux/cred.h>		// access to struct cred
#include "../../convenient.h"

#define OURMODNAME   "bad_miscdrv"
MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LLKD book:ch12/bad_miscdrv_rdwr: simple misc char driver"
" with a 'secret' to read/write AND a (contrived) privesc!");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int ga, gb = 1; /* ignore for now ... */

/*
 * The driver 'context' (or private) data structure;
 * all relevant 'state info' reg the driver is here.
 */
struct drv_ctx {
	struct device *dev;
	int tx, rx, err, myword;
	u32 config1, config2;
	u64 config3;
#define MAXBYTES    128   /* Must match the userspace app; we should actually
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
 * the number of bytes read or written on success, 0 on EOF and -1 (-ve errno)
 * on failure; here, we copy the 'secret' from our driver context structure
 * to the userspace app.
 */
static ssize_t read_miscdrv_rdwr(struct file *filp, char __user *ubuf,
				size_t count, loff_t *off)
{
	int ret = count, secret_len = strnlen(ctx->oursecret, MAXBYTES);
	struct device *dev = ctx->dev;
	void *kbuf = NULL;
	void *new_dest = NULL;

	PRINT_CTX();
	dev_info(dev, "%s wants to read (upto) %zd bytes\n", current->comm, count);

	ret = -EINVAL;
	if (count < MAXBYTES) {
		dev_warn(dev, "request # of bytes (%zd) is < required size (%d), aborting read\n",
			count, MAXBYTES);
		goto out_notok;
	}
	if (secret_len <= 0) {
		dev_warn(dev, "whoops, something's wrong, the 'secret' isn't"
			" available..; aborting read\n");
		goto out_notok;
	}

	ret = -ENOMEM;
	kbuf = kvmalloc(count, GFP_KERNEL);
	if (unlikely(!kbuf))
		goto out_nomem;

	/*
	 * In a 'real' driver, we would now actually read the content of the
	 * device hardware (or whatever) into the user supplied buffer 'ubuf'
	 * for 'count' bytes, and then copy it to the userspace process (via
	 * the copy_to_user() routine).
	 * (FYI, the copy_to_user() routine is the *right* way to copy data from
	 * userspace to kernel-space; the parameters are:
	 *  'to-buffer', 'from-buffer', count
	 *  Returns 0 on success, i.e., non-zero return implies an I/O fault).
	 * Here, we simply copy the content of our context structure's 'secret'
	 * member to userspace.
	 */

#define READ_BUG
//#undef READ_BUG
#ifdef READ_BUG
	/*
	 * As a demo of misusing the copy_to_user(), we change the destination
	 * pointer to point 512 KB *beyond* the userspace buffer; this will/could
	 * result in a bad read
	 */
	new_dest = ubuf+(512*1024);
#else
	new_dest = ubuf;
#endif
	ret = -EFAULT;
	dev_info(dev, "dest addr = %px\n", new_dest);

	if (copy_to_user(new_dest, ctx->oursecret, secret_len)) {
		dev_warn(dev, "copy_to_user() failed\n");
		goto out_ctu;
	}
	ret = secret_len;

	// Update stats
	ctx->tx += secret_len; // our 'transmit' is wrt this driver
	dev_info(dev, " %d bytes read, returning... (stats: tx=%d, rx=%d)\n",
			secret_len, ctx->tx, ctx->rx);
out_ctu:
	kvfree(kbuf);
out_nomem:
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
	int ret = count;
	struct device *dev = ctx->dev;
	void *kbuf = NULL;
	void *new_dest = NULL;

	PRINT_CTX();
	if (unlikely(count > MAXBYTES)) {   /* paranoia */
		dev_warn(dev, "count %zd exceeds max # of bytes allowed, "
			"aborting write\n", count);
		goto out_nomem;
	}
	dev_info(dev, "%s wants to write %zd bytes to (original) ubuf = %px\n",
			current->comm, count, ubuf);

	ret = -ENOMEM;
	kbuf = kvmalloc(count, GFP_KERNEL);
	if (unlikely(!kbuf))
		goto out_nomem;
	memset(kbuf, 0, count);

#define DANGER_GETROOT_BUG
//#undef DANGER_GETROOT_BUG
#ifdef DANGER_GETROOT_BUG
	/*
	 * *DANGEROUS BUG*
	 * Make the destination of the copy_from_user() point to the current
	 * process context's (real) UID; this way, we redirect the driver to
	 * write zero's here. Why? Simple: traditionally, a UID == 0 is what
	 * defines root capability!
	 */
	new_dest = &current->cred->uid;
	count = 4; /* change count as we're only updating a 32-bit quantity */
	dev_info(dev, " [current->cred=%px]\n", current->cred);
#else
	new_dest = kbuf;
#endif

	/* Copy in the user supplied buffer 'ubuf' - the data content to write -
	 * via the copy_from_user() macro.
	 * (FYI, the copy_from_user() macro is the *right* way to copy data from
	 * kernel-space to userspace; the parameters are:
	 *  'to-buffer', 'from-buffer', count
	 *  Returns 0 on success, i.e., non-zero return implies an I/O fault).
	 */
	dev_info(dev, "dest addr = %px count=%zd\n", new_dest, count);

	ret = -EFAULT;
	if (copy_from_user(new_dest, ubuf, count)) {
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
	ctx->rx += count; // our 'receive' is wrt this driver

	ret = count;
	dev_info(dev, " %zd bytes written, returning... (stats: tx=%d, rx=%d)\n",
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

	PRINT_CTX(); // displays process (or intr) context info
	ga--; gb++;
	dev_info(dev, " filename: \"%s\"\n", file_path(filp, buf, PATH_MAX));
	kfree(buf);

	return 0;
}

/* The driver 'functionality' is encoded via the fops */
static const struct file_operations llkd_misc_fops = {
	.open = open_miscdrv_rdwr,
	.read = read_miscdrv_rdwr,
	.write = write_miscdrv_rdwr,
	.llseek = no_llseek,             // dummy, we don't support lseek(2)
	.release = close_miscdrv_rdwr,
	/* As you learn more reg device drivers, you'll realize that the
	 * ioctl() would be a very useful method here. As an exercise,
	 * implement an ioctl method; when issued with the 'GETSTATS' 'command',
	 * it should return the statistics (tx, rx, errors) to the calling app
	 */
};

static struct miscdevice llkd_miscdev = {
	.minor = MISC_DYNAMIC_MINOR, // kernel dynamically assigns a free minor#
	.name = "bad_miscdrv",		/* when misc_register() is invoked, the kernel
		 * will auto-create device file as /dev/llkd_miscdrv_rdwr;
		 *  also populated within /sys/class/misc/ and /sys/devices/virtual/misc/ */
	.mode = 0666,				/* ... dev node perms set as specified here */
	.fops = &llkd_misc_fops,	/* connect to this driver's 'functionality' */
};

static int __init bad_miscdrv_init(void)
{
	int ret;
	struct device *dev;

	ret = misc_register(&llkd_miscdev);
	if (ret) {
		pr_notice("misc device registration failed, aborting\n");
		return ret;
	}
	dev = llkd_miscdev.this_device;
	dev_info(dev, "LLKD 'bad' misc driver (major # 10) registered, minor# = %d\n",
		llkd_miscdev.minor);

	/*
	 * A 'managed' kzalloc(): use the 'devres' API devm_kzalloc() for mem
	 * alloc; why? as the underlying kernel devres framework will take care of
	 * freeing the memory automatically upon driver 'detach' or when the driver
	 * is unloaded from memory
	 */
	ctx = devm_kzalloc(dev, sizeof(struct drv_ctx), GFP_KERNEL);
	if (unlikely(!ctx))
		return -ENOMEM;

	/* Retrieve the device pointer for this device */
	ctx->dev = llkd_miscdev.this_device;
	/* Initialize the "secret" value :-) */
	strlcpy(ctx->oursecret, "initmsg", 8);
	dev_dbg(ctx->dev, "A sample print via the dev_dbg(): (bad) driver initialized\n");

	return 0;		/* success */
}

static void __exit bad_miscdrv_exit(void)
{
	misc_deregister(&llkd_miscdev);
	pr_info("LLKD (bad) misc driver deregistered, bye\n");
}

module_init(bad_miscdrv_init);
module_exit(bad_miscdrv_exit);
