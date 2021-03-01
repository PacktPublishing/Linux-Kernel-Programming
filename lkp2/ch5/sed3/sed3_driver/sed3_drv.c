/*
 * ch15/sed3/sed3_drv/sed3_drv.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 15 : Timers, kernel threads and more
 ****************************************************************
 * Brief Description:
 *
 * [Summary: 
 * This mini project is identical to sed2 except that, for the [en|de]crypt work
 * we use a workqueue (the default kernel-global workqueue) instead of manually
 * managing kthreads!]
 *
 * sed = simple encrypt decrypt (!)
 * In this 'driver', we have setup an interesting (though extremely trivial and
 * simplistic) message encrypt/decrypt facility. The idea is this: a user mode
 * app (it's in ../userapp_sed), opens this misc character driver's device file
 * (/dev/sed3_drv) and issues an ioctl(2) upon it. The ioctl() call passes
 * a data structure that encapsulates the data being passed (the 'payload'), it's
 * length, the operation (or "transform") to perform upon it, and a timed_out
 * field (to figure out if it failed due to missing it's deadline).
 * The valid ops are :
 *  encrypt : XF_ENCRYPT
 *  decrypt : XF_DECRYPT
 *
 * Associated with the operation is a deadline; it's defined as 10 milliseconds.
 * If the op takes longer, a kernel timer we've setup will expire; it will set
 * the context structure's timed_out member to 1 signifying failure. We have
 * the ability for the user mode app to receive details and interpret them.
 *
 * *** DIFFERENCE from sed2 ***
 * 1) The 'work' of encryption/decryption is now performed by our work task,
 * which is 'consumed' by the default kernel-global workqueue (internally,
 * it employs a kernel thread worker pool whose management is entirely
 * handled by the kernel).
 * 2) Of course, we now run the kernel timer within the workqueue's context and
 * show if it expires prematurely (indicating that the deadline wasn't met).
 * 3) A quick test reveals that eliminating the several pr_debug() printk's
 * within the 'critical section' goes a long way towards reducing the time taken
 * to complete the 'work'! (You can always change the Makefile's EXTRA_CFLAGS var
 * to -UDEBUG to eliminate this overhead!).
 * Hence, we keep the deadline longer (10 ms).
 * ***
 *
 * So, in a nutshell, the whole idea here is to primarily demo this driver using
 * a custom work task via the default kernel-global workqueue, along with a
 * kernel timer to timeout an operation.
 * (FYI, though we certainly could, we don't use the dev_<foo>() printk's as
 * would usually be appropriate in a driver, we simply stick with the regular
 * pr_<foo>() routines).
 *
 * Summary (high level design for this sed3 mini project):
 *
 *               User-space application
 *               /     |        |     \
 *       |-------      |        |      -------|
 *op:   encrypt      retrieve  decrypt     destroy
 *                    [ioctl]              [ioctl]
 *
 *  <------------------ sed3 driver ------------------>
 *
 *by:  [workq task]     [ioctl]  [workq task]  [ioctl]
 *         ^            ^          ^            ^
 *         |            |          |            |
 *         v            v          v            v
 *       {--------- shared memory region ---------}
 *
 * It's identical to sed2 except that, for the [en|de]crypt work we use a
 * workqueue instead of manually managing kthreads!
 * Note that the user space app is the same as in sed2.
 *
 * For details, pl refer the book, Ch 15.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <asm/atomic.h>
#include <linux/workqueue.h>
#include <linux/sched/task.h>	// {get,put}_task_struct()
#include <linux/sched/signal.h> // signal_pending()

// copy_[to|from]_user()
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 11, 0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

#include "../sed_common.h"
#include "../../../convenient.h"

#define DRVNAME			"sed3_drv"
#define TIMER_EXPIRE_MS		10 // 1
#define KTHREAD_NAME	"worker"

MODULE_DESCRIPTION(
"sed3: simple encrypt-decrypt driver; demo misc driver for kernel workqueue usage, kernel timers (and ioctl)");
MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/* Module parameters */
static int make_it_fail;
module_param(make_it_fail, int, 0660);
MODULE_PARM_DESC(make_it_fail,
"Deliberately ensure that the kernel timeout occurs before processing completes (default=0)");

/*
 * The driver 'context' (or private) data structure;
 * all relevant 'state info' reg the driver is here.
 * The @msg_state, @work_done members help as a (poor) synchronization
 * mechanism in the absence of locking.
 */
struct stMyCtx {
	struct device *dev;
	atomic_t msg_state;	// state of msg: encypted or decrypted
	atomic_t work_done;
	atomic_t timed_out;
	struct timer_list timr;
	struct work_struct sed_work;
	struct sed_ds *kdata; //*kd, *kdret;
	ktime_t t1, t2;		// a s64 qty
};
static struct stMyCtx *gpriv;

static void timesup(struct timer_list *timer)
{
	struct stMyCtx *priv = from_timer(priv, timer, timr);

	atomic_set(&priv->timed_out, 1);
	pr_notice("*** Timer expired! ***\n");
	PRINT_CTX();
}

#define WORK_IS_ENCRYPT		 1
#define WORK_IS_DECRYPT		2
#define CRYPT_OFFSET		0x3F  // 63

/*
 * encrypt_decrypt_payload - the actual work
 * TOO (Theory Of Operation):
 * The inverse of the XOR operation is the XOR !
 * a ^ x = (a ^ x) ^ x
 * So, here in the encrypt routine, we perform the first part, the (a ^ x)
 *
 * @work  : one of WORK_IS_ENCRYPT or WORK_IS_DECRYPT
 * @kd    : structure containing the payload
 */
static void encrypt_decrypt_payload(int work, struct sed_ds *kd)
{
	int i;

print_hex_dump_bytes("kdata->shmem: ", DUMP_PREFIX_OFFSET, kd->shmem, kd->len);

	// Perform the actual processing on the payload
	//memcpy(kdret, kd, sizeof(struct sed_ds));
	if (work == WORK_IS_ENCRYPT) {
		for (i = 0; i < kd->len; i++) {
			kd->shmem[i] ^= CRYPT_OFFSET;
			kd->shmem[i] += CRYPT_OFFSET;
		}
	} else if (work == WORK_IS_DECRYPT) {
		for (i = 0; i < kd->len; i++) {
			kd->shmem[i] -= CRYPT_OFFSET;
			kd->shmem[i] ^= CRYPT_OFFSET;
		}
	}
}

/*
 * sed3_worker - our work task (consumed via the kernel-global workqueue)!
 */
static void sed3_worker(struct work_struct *work)
{
	struct stMyCtx *priv = container_of(work, struct stMyCtx, sed_work);

		pr_debug("starting timer + processing now ...\n");
		/* Start - the timer; set it to expire in TIMER_EXPIRE_MS ms */
		if (mod_timer(&priv->timr, jiffies + msecs_to_jiffies(TIMER_EXPIRE_MS)))
			pr_alert("timer already active?\n");
		priv->t1 = ktime_get_real_ns();

		/*--------------- Critical section begins --------------------------*/
		pr_debug("[%d] work task about to execute work!\n", task_pid_nr(current));
		PRINT_CTX();

		atomic_set(&priv->work_done, 0);
		switch (priv->kdata->data_xform) {
		case XF_NONE:
			pr_debug("data transform type: XF_NONE\n");
			// nothing to do
			break;
		case XF_ENCRYPT:
			pr_debug("data transform type: XF_ENCRYPT\n");
			encrypt_decrypt_payload(WORK_IS_ENCRYPT, priv->kdata);
			atomic_set(&priv->msg_state, XF_ENCRYPT);
			break;
		case XF_DECRYPT:
			pr_debug("data transform type: XF_DECRYPT\n");
			encrypt_decrypt_payload(WORK_IS_DECRYPT, priv->kdata);
			atomic_set(&priv->msg_state, XF_DECRYPT);
			break;
		default:
			pr_warn("unknown transform passed (%d)\n", priv->kdata->data_xform);
		}
		atomic_set(&priv->work_done, 1);

		if (make_it_fail == 1)
			msleep(TIMER_EXPIRE_MS + 10);
		/*--------------- Critical section ends ----------------------------*/

		priv->t2 = ktime_get_real_ns();
		// work done, cancel the timeout
		if (del_timer(&priv->timr) == 0)
			pr_info("cancelled the timer while it's inactive! (deadline missed?)\n");
		else
			pr_info("processing complete, timeout cancelled\n");
		SHOW_DELTA(priv->t2, priv->t1);

		pr_info("[%d] FYI, work task done, leaving...\n",
		    task_pid_nr(current));
}

/*
 * Is our kthread performing any ongoing work right now? poll...
 * Not ideal (but we'll live with it); better to use a lock (we cover locking
 * in this book's last two chapters)
 */
#define POLL_ON_WORK_DONE(sleep_ms) do { \
		while (atomic_read(&priv->work_done) == 0) \
			msleep_interruptible(sleep_ms);  \
} while (0)

/*
 * The ioctl method for this demo driver; note how we take into
 * account the fact that the ioctl's signtaure changed from 2.6.36 (as the
 * BKL was finally removed).
 *
 * CAREFUL!
 * What if two or more threads (or processes) concurrently invoke this driver
 * ioctl() method? There will be concurrency issues! ... which we haven't taken
 * care of here... This, and a lot more, will be discussed and detailed in this
 * book's last two chapters on Kernel Synchronization, a very real and important
 * topic.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static long ioctl_miscdrv(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int ioctl_miscdrv(struct inode *ino, struct file *filp, unsigned int cmd,
			 unsigned long arg)
#endif
{
	struct stMyCtx *priv = gpriv;

	/* Verify stuff: is the ioctl's for us? etc.. */
	if (_IOC_TYPE(cmd) != IOCTL_LLKD_SED_MAGIC) {
		pr_warn("ioctl fail; magic # mismatch\n");
		return -ENOTTY;
	}
	if (_IOC_NR(cmd) > IOCTL_LLKD_SED_MAXIOCTL) {
		pr_warn("ioctl fail; invalid cmd?\n");
		return -ENOTTY;
	}

	switch (cmd) {
	case IOCTL_LLKD_SED_IOC_ENCRYPT_MSG: /* kthread: encrypts the msg passed in */
		pr_debug("In ioctl 'encrypt' cmd option; arg=0x%lx\n", arg);
#if 0	/* only allow root? here, it's just to demo that you can do stuff like
		 * this; even better, use POSIX capabilities (see capabilities(7))
		 */
		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
#endif
		if (atomic_read(&priv->msg_state) == XF_ENCRYPT) {  // already encrypted?
			pr_notice("encrypt op: message is currently encrypted; aborting op...\n");
			return -EBADRQC; /* 'Invalid request code' */
		}
		if (copy_from_user(priv->kdata, (struct sed_ds *)arg, sizeof(struct sed_ds))) {
			pr_warn("copy_from_user() failed\n");
			return -EFAULT;
		}
		pr_debug("xform=%d, len=%d\n", priv->kdata->data_xform, priv->kdata->len);
		if (priv->kdata->len == 0) {
			pr_warn("no data passed (len is 0)\n");
			return -EINVAL;
		}
		print_hex_dump_bytes("payload: ", DUMP_PREFIX_OFFSET, priv->kdata->shmem, priv->kdata->len);

		POLL_ON_WORK_DONE(1);
		/* Schedule our work task and have it encrypt the message ! */
		if (!schedule_work(&priv->sed_work))
			pr_warn("work already on the kernel-global wq?\n");

		/*
		 * Now, our kernel thread is doing the 'work'; it will either be done,
		 * or it will miss it's deadline and fail.
		 * Attempting to lookup the payload or do anything more here would be a
		 * mistake, a race! Why? We're currently running in the ioctl() process
		 * context; the kernel thread runs in it's own process context! (If we
		 * must look it up, then we really require a (mutex) lock; we shall
		 * discuss locking in detail in the book's last two chapters.
		 */
		break;
	case IOCTL_LLKD_SED_IOC_DECRYPT_MSG: /* kthread: decrypts the encrypted msg */
		pr_debug("In ioctl 'decrypt' cmd option\n");
		if (atomic_read(&priv->msg_state) == XF_DECRYPT) {   // already decrypted?
			pr_notice("decrypt op: message is currently decrypted; aborting op...\n");
			return -EBADRQC; /* 'Invalid request code' */
		}
		priv->kdata->data_xform = XF_DECRYPT;

		POLL_ON_WORK_DONE(1);
		/* Schedule our work task and have it decrypt the message ! */
		if (!schedule_work(&priv->sed_work))
			pr_warn("work already on the kernel-global wq?\n");

		break;
	case IOCTL_LLKD_SED_IOC_RETRIEVE_MSG: /* ioctl: retrieves the encrypted msg */
		pr_debug("In ioctl 'retrieve' cmd option; arg=0x%lx\n", arg);
		if (atomic_read(&priv->timed_out) == 1) {
			pr_debug("the encrypt op had timed out! retruning -ETIMEDOUT\n");
			return -ETIMEDOUT;
		}
		if (copy_to_user((struct sed_ds *)arg, (struct sed_ds *)priv->kdata, sizeof(struct sed_ds))) {
			pr_warn("copy_to_user() failed\n");
			return -EFAULT;
		}
		break;
	case IOCTL_LLKD_SED_IOC_DESTROY_MSG: /* ioctl: destroys the msg */
		pr_debug("In ioctl 'destroy' cmd option\n");
		memset(priv->kdata, 0, sizeof(struct sed_ds));
		atomic_set(&priv->msg_state, 0);
		atomic_set(&priv->work_done, 1);
		atomic_set(&priv->timed_out, 0);
		priv->t1 = priv->t2 = 0;
		break;
	default:
		return -ENOTTY;
	}
	return 0;
}

/*----------- 'misc' methods -------------
 * dummy here...
 */
static int open_miscdrv(struct inode *inode, struct file *filp)
{
	PRINT_CTX();		// displays process (or atomic) context info
	pr_info("opening \"%s\" now\n", filp->f_path.dentry->d_iname);

	return nonseekable_open(inode, filp);
}

static ssize_t read_miscdrv(struct file *filp, char __user *ubuf, size_t count, loff_t *off)
{
	pr_info("to read %zd bytes\n", count);
	return count;
}

static ssize_t write_miscdrv(struct file *filp, const char __user *ubuf,
			     size_t count, loff_t *off)
{
	pr_info("to write %zd bytes\n", count);
	return count;
}

static int close_miscdrv(struct inode *inode, struct file *filp)
{
	pr_info("closing \"%s\"\n", filp->f_path.dentry->d_iname);
	return 0;
}

/*-------------- For registration with a kernel framework ---------------------*/
// Here, we shall register ourselves with the 'misc' kernel framework

/* The driver 'functionality' is encoded via the fops */
static const struct file_operations llkd_misc_fops = {
	.open = open_miscdrv,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	.unlocked_ioctl = ioctl_miscdrv,	// use the 'unlocked' version
#else
	.ioctl = ioctl_miscdrv,	// 'old' way
#endif
	.read = read_miscdrv,
	.write = write_miscdrv,
	.llseek = no_llseek,	// dummy, we don't support lseek(2)
	.release = close_miscdrv,
};

static struct miscdevice llkd_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,	/* kernel dynamically assigns a free minor# */
	.name = DRVNAME,	/* when misc_register() is invoked, the kernel
				 * will auto-create device file as /dev/llkd_miscdrv_rdwr;
				 *  also populated within /sys/class/misc/ and /sys/devices/virtual/misc/ */
	.mode = 0666,		/* ... dev node perms set as specified here */
	.fops = &llkd_misc_fops,	/* connect to this driver's 'functionality' */
};

static int __init sed3_drv_init(void)
{
	int ret = 0;
	struct device *dev;
	struct stMyCtx *priv = NULL;

	//--- Register with misc kernel framework
	ret = misc_register(&llkd_miscdev);
	if (ret) {
		pr_notice("misc device registration failed, aborting\n");
		return ret;
	}
	dev = llkd_miscdev.this_device;

	dev_info(dev, "LLKD %s misc driver (major # 10) registered, minor# = %d,\n"
		 "dev node is /dev/%s\n", DRVNAME, llkd_miscdev.minor, DRVNAME);

	/* We're using a global pointer to our private context structure. In a
	 * "real" driver, you'd typically hook up your private structure pointer
	 * to an appropriate member within the bus driver's structure(s);
	 * f.e., we could base this on a platform bus and do
	 *   plat0.dev.platform_data = priv;
	 * Here, for simplicity, we avoid attaching to a bus driver at all and
	 * simply employ the misc framework. This then implies that we access a
	 * global without explicit protection in various parts of this code; this
	 * issue is completely addressed in the next two chapters on kernel
	 * synchronization; here we simply ignore it.
	 */
	gpriv = priv = devm_kzalloc(dev, sizeof(struct stMyCtx), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	priv->dev = llkd_miscdev.this_device;
	atomic_set(&priv->work_done, 1);

	gpriv->kdata = devm_kzalloc(dev, sizeof(struct sed_ds), GFP_KERNEL);
	if (!gpriv->kdata)
		return -ENOMEM;

	INIT_WORK(&priv->sed_work, sed3_worker);
	pr_info("Our work task on the kernel-global workqueue is initialized\n");

	// Initialize our kernel timer
	timer_setup(&priv->timr, timesup, 0);

	pr_info("init done (make_it_fail is %s)\n", make_it_fail == 1 ? "*on*" : "off");
	dev_dbg(dev, "loaded.\n");
	return ret;
}

static void __exit sed3_drv_exit(void)
{
	struct stMyCtx *priv = gpriv;

	pr_debug("waiting for our work task to finish...\n");
	if (cancel_work_sync(&priv->sed_work))
		pr_info("yes, there was indeed some pending work; now done...\n");
	dev_dbg(priv->dev, "unloading\n");
	del_timer_sync(&priv->timr);
	misc_deregister(&llkd_miscdev);
}

module_init(sed3_drv_init);
module_exit(sed3_drv_exit);
