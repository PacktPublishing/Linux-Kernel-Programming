/*
 * solutions_to_assgn/ch10/miscdrv_rdwr_mutextrylock/miscdrv_rdwr_mutextrylock.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch 10 : Synchronization Primitives and How to Use Them
 ****************************************************************
 * Assignment 10.1 :
 * Modify the ch10/1_miscdrv_rdwr_mutexlock/miscdrv_rdwr_mutexlock.c device
 * driver to use the *trylock* mutex variant in the driver's write() method.
 * If the lock is not acquired, perform a hex dump of the driver 'context'
 * structure ('busy-loop' over this).
 *
 * For details, please refer the book, Ch 10.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>         // k[m|z]alloc(), k[z]free(), ...
#include <linux/mm.h>           // kvmalloc()
#include <linux/fs.h>		// the fops structure
#include <linux/uaccess.h>      // copy_to|from_user() macros
#include <linux/mutex.h>
#include "../../../convenient.h"

#define OURMODNAME   "miscdrv_rdwr_mutextrylock"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKDC book:solutions_to_assgn/ch10/miscdrv_rdwr_mutextrylock:"
		" a solution to Assignment 10.1");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int ga, gb = 1;
DEFINE_MUTEX(lock1); // protects the global integers ga and gb

/* The driver 'context' data structure;
 * all relevant 'state info' reg the driver is here.
 */
struct drv_ctx {
	int tx, rx, err, myword;
	u32 config1, config2;
	u64 config3;
#define MAXBYTES    128
	char oursecret[MAXBYTES];
	struct mutex lock;  // protects this data structure
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
	PRINT_CTX(); // displays process (or intr) context info

	mutex_lock(&lock1);
	ga ++; gb --;
	mutex_unlock(&lock1);

	pr_info("%s:%s():\n"
		" filename: \"%s\"\n"
		" wrt open file: f_flags = 0x%x\n"
		" ga = %d, gb = %d\n",
	       OURMODNAME, __func__, filp->f_path.dentry->d_iname,
	       filp->f_flags, ga, gb);

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
	int ret = count, secret_len;

	mutex_lock(&ctx->lock);
	secret_len = strlen(ctx->oursecret);
	mutex_unlock(&ctx->lock);

	PRINT_CTX();
	pr_info("%s:%s():\n %s wants to read (upto) %ld bytes\n",
			OURMODNAME, __func__, current->comm, count);

	ret = -EINVAL;
	if (count < MAXBYTES) {
		pr_warn("%s:%s(): request # of bytes (%ld) is < required size (%d), aborting read\n",
				OURMODNAME, __func__, count, MAXBYTES);
		goto out_notok;
	}
	if (secret_len <= 0) {
		pr_warn("%s:%s(): whoops, something's wrong, the 'secret' isn't"
			" available..; aborting read\n",
				OURMODNAME, __func__);
		goto out_notok;
	}

	/* In a 'real' driver, we would now actually read the content of the
	 * device hardware (or whatever) into the user supplied buffer 'ubuf'
	 * for 'count' bytes, and then copy it to the userspace process (via
	 * the copy_to_user() macro).
	 * (FYI, the copy_to_user() macro is the *right* way to copy data from
	 * userspace to kernel-space; the parameters are:
	 *  'to-buffer', 'from-buffer', count
	 *  Returns 0 on success, i.e., non-zero return implies an I/O fault).
	 * Here, we simply copy the content of our context structure's 'secret'
	 * member to userspace.
	 */
	ret = -EFAULT;
	mutex_lock(&ctx->lock);
	if (copy_to_user(ubuf, ctx->oursecret, secret_len)) {
		pr_warn("%s:%s(): copy_to_user() failed\n", OURMODNAME, __func__);
		goto out_ctu;
	}
	ret = secret_len;

	// Update stats
	ctx->tx += secret_len; // our 'transmit' is wrt this driver
	pr_info(" %d bytes read, returning... (stats: tx=%d, rx=%d)\n",
			secret_len, ctx->tx, ctx->rx);
out_ctu:
	mutex_unlock(&ctx->lock);
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
	int ret, tl = 0, try = 1;
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
		pr_warn("%s:%s(): copy_from_user() failed\n", OURMODNAME, __func__);
		goto out_cfu;
	}

	/* In a 'real' driver, we would now actually write (for 'count' bytes)
	 * the content of the 'ubuf' buffer to the device hardware (or whatever),
	 * and then return.
	 *
	 * Here, using the *trylock* variant, we *attempt* to acquire the mutex
	 * lock; if we don't get it, we perform some 'work'. This is done in a
	 * loop... When we do get the lock, then write the just-accepted
	 * new 'secret' into our driver 'context' structure, and unlock.
	 *
	 * The likely scenario: this lock is of course lightly contended (we
	 * don't really have many threads competing for it - unless you write
	 * a test case for that); hence, we will likely acquire the mutex on
	 * the first attempt.
	 */
	while (0 == (tl = mutex_trylock(&ctx->lock))) { // not acquired the lock
		pr_info("%s:%s(): try #%d: mutex trylock NOT acquired ...\n",
			OURMODNAME, __func__, try ++);
		DELAY_LOOP('L', 72);
#if 1
		/* Pedantically wrong; we must have the lock when reading the
		 * 'ctx' structure */
		print_hex_dump_bytes("ctx ", DUMP_PREFIX_OFFSET,
				ctx, sizeof(struct drv_ctx));
#endif
	}
	if (1 == tl) {      // acquired the lock!
		pr_info("%s:%s(): try #%d: mutex trylock acquired ...\n",
			OURMODNAME, __func__, try ++);
		strlcpy(ctx->oursecret, kbuf, (count > MAXBYTES ? MAXBYTES : count));
		// Update stats
		ctx->rx += count; // our 'receive' is wrt this driver

		ret = count;
		pr_info(" %ld bytes written, returning... (stats: tx=%d, rx=%d)\n",
			count, ctx->tx, ctx->rx);
		mutex_unlock(&ctx->lock);
	}
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

	mutex_lock(&lock1);
	ga --; gb ++;
	mutex_unlock(&lock1);

        /* REQD:: XXX : spin_lock(filp->f_lock); .. then unlock 
         *  do this for the CORRECT drv; miscdrv_enh.ko */
        pr_info("%s:%s(): filename: \"%s\"\n"
		" ga = %d, gb = %d\n",
			OURMODNAME, __func__, filp->f_path.dentry->d_iname,
			ga, gb);
        return 0;
}

/* The driver 'functionality' is encoded via the fops */
static const struct file_operations lkdc_misc_fops = {
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

static struct miscdevice lkdc_miscdev = {
	.minor = MISC_DYNAMIC_MINOR, // kernel dynamically assigns a free minor#
	.name = "lkdc_miscdrv_mutextrylock",
	    // populated within /sys/class/misc/ and /sys/devices/virtual/misc/
	.fops = &lkdc_misc_fops,     // connect to 'functionality'
};

static int __init miscdrv_init_mutextrylock(void)
{
	int ret;

	if ((ret = misc_register(&lkdc_miscdev))) {
		pr_notice("%s: misc device registration failed, aborting\n",
			       OURMODNAME);
		return ret;
	}
	pr_info("%s: LKDC misc driver (major # 10) registered, minor# = %d\n",
			OURMODNAME, lkdc_miscdev.minor);

	/* Now, for the purpose of creating the device node (file), we require
	 * both the major and minor numbers. The major number will always be 10
	 * (it's reserved for all 'misc' class devices). Reg the minor number's
	 * retrieval, here's one (rather silly) technique:
	 * Write the minor # into the kernel log in an easily grep-able way (so
	 * that we can do a
	 *  MINOR=$(dmesg |grep "^miscdrv_rdwr\:minor=" |cut -d"=" -f2)
	 * from a shell script!). Of course, this approach is silly; in the
	 * real world, superior techniques (typically 'udev') are used.
	 * Here, we do provide a utility script (cr8devnode.sh) to do this and create the
	 * device node.
	 */
	pr_info("%s:minor=%d\n", OURMODNAME, lkdc_miscdev.minor);

	ctx = kzalloc(sizeof(struct drv_ctx), GFP_KERNEL);
	if (unlikely(!ctx)) {
		pr_notice("%s: kzalloc failed! aborting\n", OURMODNAME);
		return -ENOMEM;
	}
	mutex_init(&ctx->lock);
	strlcpy(ctx->oursecret, "initmsg", 8);
		/* Why don't we protect the above strlcpy() with the mutex lock?
		 * It's working on shared writable data, yes?
		 * No; this is the init code; it's guaranteed to run in exactly
		 * one context (typically the insmod(8) process), thus there is
		 * no concurrency possible here. The same goes for the cleanup
		 * code path.
		 */

	return 0;		/* success */
}

static void __exit miscdrv_exit_mutextrylock(void)
{
	mutex_destroy(&lock1);
	mutex_destroy(&ctx->lock);
	kzfree(ctx);
	misc_deregister(&lkdc_miscdev);
	pr_debug("%s: LKDC misc driver deregistered, bye\n", OURMODNAME);
}

module_init(miscdrv_init_mutextrylock);
module_exit(miscdrv_exit_mutextrylock);
