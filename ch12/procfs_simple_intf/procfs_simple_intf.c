/*
 * ch12/procfs_simple_intf/procfs_simple_intf.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 12 : User-Kernel communication pathways
 ****************************************************************
 * Brief Description:
 * Simple kernel module to demo interfacing with userspace via procfs.
 * Here, we issue appropriate kernel APIs to have the procfs layer ceeate this
 * tree for us:
 * /proc
 *  ...
 *  |---procfs_simple_intf           <-- our proc folder
 *      |---llkdproc_config1
 *      |---llkdproc_show_pgoff
 *      |---llkdproc_dbg
 *
 * llkdproc_config1    : RW
 *                       W: write a value to drvctx->config1;
 *	                 R: read retrieves (to userspace) the current value
 *                          of drvctx->config1
 * llkdproc_show_pgoff : R-
 *			 R: read retrieves (to userspace) the value of PAGE_OFFSET
 * llkdproc_show_dbg   : R-
 *			 R: read retrieves (to userspace) the values in the
 *                          driver context data structure
 *
 * For details, please refer the book, Ch 12.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>  /* procfs APIs etc */
#include <linux/seq_file.h>

// copy_[to|from]_user()
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,11,0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LLKD book:ch12/procfs_simple_intf: simple procfs interfacing demo");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

#define	OURMODNAME	"procfs_simple_intf"
#define	PROC_FILE1	"llkdproc_config1"
#define	PROC_FILE2	"llkdproc_show_pgoff"
#define	PROC_FILE3	"llkdproc_show_dbg"

//--- our MSG() macro
#ifdef DEBUG
#define MSG(string, args...)  do {                                   \
	pr_info("%s:%s():%d: " string,                               \
			OURMODNAME, __FUNCTION__, __LINE__, ##args); \
} while(0)
#else
#define MSG(string, args...)
#endif

DEFINE_MUTEX(mtx);

/* Borrowed from ch11; the 'driver context' data structure;
 * all relevant 'state info' reg the driver is here.
 */
struct drv_ctx {
	int tx, rx, err, myword, power;
	u32 config1, config2;
	u64 config3;
#define MAXBYTES   128
	char oursecret[MAXBYTES];
};
static struct drv_ctx *gdrvctx = NULL;

/*-------------------- proc callbacks follow */


#if 0
/* Display something..
 * Max of page size bytes with a single seq_printf call.
 */
static int myproc_show(struct seq_file *seq, void *v)
{
	MSG("\n");
	if (mutex_lock_interruptible(&mtx)) {
		return -ERESTARTSYS;
	}
	seq_printf(seq, "%s:gbuf:%s\n(fyi, page size = %ld bytes)\n",
		   OURMODNAME, gbuf, PAGE_SIZE);
	mutex_unlock(&mtx);
	return 0;
}
static ssize_t myproc_write(struct file *file, const char __user * user,
			    size_t count, loff_t * data)
{
	char opstr[4];
	int ret = count, n = 20;

	MSG("\n");
	if (mutex_lock_interruptible(&mtx)) {
		ret = -ERESTARTSYS;
	}
	if (count == 0 || count > sizeof(opstr)) {
		ret = -EINVAL;
		goto out;
	}
	if (copy_from_user(opstr, user, count)) {
		ret = -EFAULT;
		goto out;
	}
	opstr[count - 1] = '\0';

	if (!strcmp(opstr, "on")) {
		snprintf(gbuf, MAXLEN - 1, "ON:LLLLL:%d:%d", MAXLEN, n);
	} else if (!strcmp(opstr, "off")) {
		snprintf(gbuf, MAXLEN - 1, "OFF:!!!!!:%d:%d", MAXLEN, n);
	} else {
		pr_info("%s:%s:only values of 'on' or 'off' are valid",
			OURMODNAME, __FUNCTION__);
		ret = -EINVAL;
		goto out;
	}
 out:
	mutex_unlock(&mtx);
	return ret;
}
#endif

/* Displays the system PAGE_OFFSET value */
static int proc_show_pgoff(struct seq_file *seq, void *v)
{
	seq_printf(seq, "%s:PAGE_OFFSET:0x%lx\n", OURMODNAME, PAGE_OFFSET);
	return 0;
}

/* Display the driver context 'config1' value */
static int proc_show_config1(struct seq_file *seq, void *v)
{
	if (mutex_lock_interruptible(&mtx)) {
		return -ERESTARTSYS;
	}
	seq_printf(seq, "%s:config1:%d,0x%x\n",
		OURMODNAME, gdrvctx->config1, gdrvctx->config1);
	mutex_unlock(&mtx);
	return 0;
}

/* Display the driver context 'config1' value */
static ssize_t myproc_write_config1(struct file *filp, const char __user *ubuf,
				size_t count, loff_t *off)
{
	char buf[8];
	int ret = count;
	unsigned long configval=0;

	if (mutex_lock_interruptible(&mtx)) {
		return -ERESTARTSYS;
	}
	if (count == 0 || count > sizeof(buf)) {
		ret = -EINVAL;
		goto out;
	}
	if (copy_from_user(buf, ubuf, count)) {
		ret = -EFAULT;
		goto out;
	}
	buf[count - 1] = '\0';
	MSG("user sent: buf = %s\n", buf);
	ret = kstrtoul(buf, 0, &configval);
	if (ret)
		goto out;
	gdrvctx->config1 = configval;
	ret = count;
out:
	mutex_unlock(&mtx);
	return ret;
}

static int myproc_open_config1(struct inode *inode, struct file *file)
{
        return single_open(file, proc_show_config1, NULL);
}

static const struct file_operations myproc_fops_config1 = {
	.owner = THIS_MODULE,
	.open = myproc_open_config1,
	.read = seq_read,
	.write = myproc_write_config1,
	.llseek = seq_lseek,
	.release = single_release,	/* builtin */
};
#if 0
static const struct file_operations myproc_fops_dbg = {
	.owner = THIS_MODULE,
	.open = myproc_open,
	.read = seq_read,
	.write = myproc_write,
	.llseek = seq_lseek,
	.release = single_release,	/* builtin */
};
#endif

static struct drv_ctx *alloc_init_drvctx(void)
{
	struct drv_ctx *drvctx = NULL;
	drvctx = kzalloc(sizeof(struct drv_ctx), GFP_KERNEL);
	if (!drvctx) {
		pr_warn("%s: kzalloc failed\n", OURMODNAME);
		return ERR_PTR(-ENOMEM);
	}
	drvctx->power = 1;
	MSG("allocated and init the driver context structure\n");
	return drvctx;
}

static struct proc_dir_entry *gprocdir;

static int __init procfs_simple_intf_init(void)
{
	int stat = 0;

#ifndef	CONFIG_PROC_FS
	pr_warn("%s: procfs unsupported! Aborting ...\n", OURMODNAME);
	return -EINVAL;
#endif
	gdrvctx = alloc_init_drvctx();
	if (IS_ERR(gdrvctx)) {
		pr_warn("%s: drv ctx alloc failed, aborting...\n", OURMODNAME);
		stat = PTR_ERR(gdrvctx);
		goto out_fail_1;
	}

	/* 1. Create our parent dir under /proc called OURMODNAME */
	gprocdir = proc_mkdir(OURMODNAME, NULL);
	if (!gprocdir) {
		pr_warn("%s: proc_mkdir failed, aborting...\n", OURMODNAME);
		stat = -ENOMEM;
		goto out_fail_2;
	}
	MSG("proc dir (/proc/%s) created\n", OURMODNAME); 

	/* 2. Create the PROC_FILE1 proc entry under the parent dir OURMODNAME;
	   this will serve as the 'read/write drv_ctx->config1' (pseudo) file */
	if (!proc_create(PROC_FILE1, 0640, gprocdir, &myproc_fops_config1)) {
		pr_warn("%s: proc_create [1] failed, aborting...\n", OURMODNAME);
		stat = -ENOMEM;
		goto out_fail_3;
	}
	MSG("proc file 1 (/proc/%s/%s) created\n", OURMODNAME, PROC_FILE1);

	/* 3. Create the PROC_FILE2 proc entry under the parent dir OURMODNAME;
	   this will serve as the 'show PAGE_OFFSET' (pseudo) file;
	   as it serves precisely one purpose, we use the convenience API
	   proc_create_single_data() to create this entry */
	if (!proc_create_single_data(PROC_FILE2, 0644, gprocdir,
					proc_show_pgoff, 0)) {
		pr_warn("%s: proc_create [2] failed, aborting...\n", OURMODNAME);
		stat = -ENOMEM;
		goto out_fail_3;
	}
	MSG("proc file 2 (/proc/%s/%s) created\n", OURMODNAME, PROC_FILE2);


	pr_info("%s initialized\n", OURMODNAME);
	return 0;		/* success */

 out_fail_3:
	remove_proc_subtree(OURMODNAME, NULL);
 out_fail_2:
	kfree(gdrvctx);
 out_fail_1:
	return stat;
}

static void __exit procfs_simple_intf_cleanup(void)
{
	kfree(gdrvctx);
	remove_proc_subtree(OURMODNAME, NULL);
	pr_info("%s removed\n", OURMODNAME);
}

module_init(procfs_simple_intf_init);
module_exit(procfs_simple_intf_cleanup);
