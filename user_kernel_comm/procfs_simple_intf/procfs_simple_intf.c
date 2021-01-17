/*
 * user_kernel_comm/procfs_simple_intf/procfs_simple_intf.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch - User-Kernel communication pathways
 ****************************************************************
 * Brief Description:
 * Simple kernel module to demo interfacing with userspace via procfs.
 * Procfs is one of several available user<->kernel interfaces; the others
 * include sysfs, debugfs, netlink sockets and the ioctl.
 * In order to demonstrate (and let you easily contrast) between these
 * user<->kernel interfaces, in all cases we create four 'files' or 'objects'
 * (by issuing the appropriate kernel APIs).
 * In this particular case, the interface is via procfs, so we create four
 * procfs 'objects' - pseudo-files - under a directory whose name is the name
 * given to this kernel module. These four procfs 'files', what they are named
 * and meant for is summarized below:
 * /proc
 *  ...
 *  |---procfs_simple_intf           <-- our proc directory
 *      |---llkdproc_dbg_level
 *      |---llkdproc_show_pgoff
 *      |---llkdproc_show_drvctx
 *      |---llkdproc_config1
 *
 * Summary of our proc files and how they can be used (R=>read,W=>write)
 * (1) llkdproc_dbg_level   : RW
 *      R: read retrieves (to userspace) the current value of the global var
 *         debug_level
 *      W: write a value (from userspace) to the global var debug_level, thus
 *         changing the debug level verbosity
 *      file perms: 0644
 * (2) llkdproc_show_pgoff  : R-
 *      R: read retrieves (to userspace) the value of PAGE_OFFSET
 *      file perms: 0444
 * (3) llkdproc_show_drvctx : R-
 *      R: read retrieves (to userspace) the values in our sample 'driver
 *         context' data structure
 *      file perms: 0440
 * (4) llkdproc_config1     : RW
 *      R: read retrieves (to userspace) the current value of the 'driver
 *         context' variable drvctx->config1
 *      W: write a value (from userspace) to drvctx->config1
 *      file perms: 0644
 *  (for fun, we treat this value 'config1' as also representing the
 *   driver debug_level)
 *
 * For details, please refer the book.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>  /* procfs APIs etc */
#include <linux/seq_file.h>

// copy_[to|from]_user()
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 11, 0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LLKD book:user_kernel_comm/procfs_simple_intf: simple procfs interfacing demo");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

#define	OURMODNAME		"procfs_simple_intf"
#define	PROC_FILE1		"llkdproc_debug_level"
#define	PROC_FILE1_PERMS	0644
#define	PROC_FILE2		"llkdproc_show_pgoff"
#define	PROC_FILE2_PERMS	0444
#define	PROC_FILE3		"llkdproc_show_drvctx"
#define	PROC_FILE3_PERMS	0440
#define	PROC_FILE4		"llkdproc_config1"
#define	PROC_FILE4_PERMS	0644

/* We use a mutex lock here; details in Ch 16 and Ch 17 */
DEFINE_MUTEX(mtx);

/* Borrowed from ch12; the 'driver context' data structure:
 * all relevant 'state info' reg the driver and (fictional) 'device'
 * is maintained here.
 */
struct drv_ctx {
	int tx, rx, err, myword, power;
	u32 config1; /* treated as equivalent to 'debug level' of our driver */
	u32 config2;
	u64 config3;
#define MAXBYTES   128
	char oursecret[MAXBYTES];
};
static struct drv_ctx *gdrvctx;
static int debug_level; /* 'off' (0) by default ... */

/*------------------ proc file 4 -------------------------------------*/
/* Our proc file 4: displays the current driver context 'config1' value */
static int proc_show_config1(struct seq_file *seq, void *v)
{
	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;
	seq_printf(seq, "%s:config1:%d,0x%x\n",
		OURMODNAME, gdrvctx->config1, gdrvctx->config1);
	mutex_unlock(&mtx);
	return 0;
}

/* proc file 4 : modify the driver context 'config1' value as per what
   userspace writes */
static ssize_t myproc_write_config1(struct file *filp, const char __user *ubuf,
				size_t count, loff_t *off)
{
	char buf[8];
	int ret = count;
	unsigned long configval = 0;

	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;
	if (count == 0 || count > sizeof(buf)) {
		ret = -EINVAL;
		goto out;
	}
	if (copy_from_user(buf, ubuf, count)) {
		ret = -EFAULT;
		goto out;
	}
	buf[count - 1] = '\0';
	pr_debug("user sent: buf = %s\n", buf);
	ret = kstrtoul(buf, 0, &configval);
	if (ret)
		goto out;
	gdrvctx->config1 = configval;
	/* As we're treating 'config1' as the 'debug level', update it */
	debug_level = configval;
	ret = count;
out:
	mutex_unlock(&mtx);
	return ret;
}

static int myproc_open_config1(struct inode *inode, struct file *file)
{
	return single_open(file, proc_show_config1, NULL);
}

static const struct file_operations fops_rdwr_config1 = {
	.owner = THIS_MODULE,
	.open = myproc_open_config1,
	.read = seq_read,
	.write = myproc_write_config1,
	.llseek = seq_lseek,
	.release = single_release,
};

/*------------------ proc file 3 -------------------------------------*/
/* Our proc file 3: displays the 'driver context' data structure */
static int proc_show_drvctx(struct seq_file *seq, void *v)
{
	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;
	seq_printf(seq, "prodname:%s\n"
			"tx:%d,rx:%d,err:%d,myword:%d,power:%d\n"
			"config1:0x%x,config2:0x%x,config3:0x%llx\n"
			"oursecret:%s\n",
	OURMODNAME,
	gdrvctx->tx, gdrvctx->rx, gdrvctx->err, gdrvctx->myword, gdrvctx->power,
	gdrvctx->config1, gdrvctx->config2, gdrvctx->config3,
	gdrvctx->oursecret);
	mutex_unlock(&mtx);
	return 0;
}

static int myproc_open_drvctx(struct inode *inode, struct file *file)
{
	return single_open(file, proc_show_drvctx, NULL);
}

static const struct file_operations fops_show_drvctx = {
	.owner = THIS_MODULE,
	.open = myproc_open_drvctx,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct drv_ctx *alloc_init_drvctx(void)
{
	struct drv_ctx *drvctx = NULL;

	drvctx = kzalloc(sizeof(struct drv_ctx), GFP_KERNEL);
	if (!drvctx)
		return ERR_PTR(-ENOMEM);
	/* initialize our 'driver context' with something... */
	drvctx->config1 = 0x0;
	drvctx->config2 = 0x48524a5f;
	drvctx->config3 = 0x424c0a52;
	drvctx->power = 1;
	strncpy(drvctx->oursecret, "AhA xxx", 8);

	pr_debug("allocated and init the driver context structure\n");
	return drvctx;
}

/*------------------ proc file 2 -------------------------------------*/
/* Our proc file 2: displays the system PAGE_OFFSET value */
static int proc_show_pgoff(struct seq_file *seq, void *v)
{
	seq_printf(seq, "%s:PAGE_OFFSET:0x%px\n", OURMODNAME, PAGE_OFFSET);
	return 0;
}

/*------------------ proc file 1 -------------------------------------*/
#define DEBUG_LEVEL_MIN		0
#define DEBUG_LEVEL_MAX		2
#define DEBUG_LEVEL_DEFAULT	DEBUG_LEVEL_MIN

/* proc file 1 : modify the driver's debug_level global variable as per
 * what userspace writes */
static ssize_t myproc_write_debug_level(
		struct file *filp, const char __user *ubuf, size_t count,
		loff_t *off)
{
	char buf[12];
	int ret = count;

	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;
	if (count == 0 || count > 12) {
		ret = -EINVAL;
		goto out;
	}
	/* Get the usermode buffer content into the kernel (into 'buf') */
	if (copy_from_user(buf, ubuf, count)) {
		ret = -EFAULT;
		goto out;
	}
	buf[count - 1] = '\0';
	pr_debug("user sent: buf = %s\n", buf);
	ret = kstrtoint(buf, 0, &debug_level); /* update it! */
	if (ret)
		goto out;
	if (debug_level < DEBUG_LEVEL_MIN || debug_level > DEBUG_LEVEL_MAX) {
		pr_info("trying to set invalid value for debug_level\n"
			" [allowed range: %d-%d]\n", DEBUG_LEVEL_MIN, DEBUG_LEVEL_MAX);
		debug_level = DEBUG_LEVEL_DEFAULT;
		ret = -EFAULT;
		goto out;
	}

// eg. of using dyn dbg level
// MSG_L1 , MSG_L2 , ...

	/* just for fun, lets say that our drv ctx 'config1'
	   represents the debug level */
	gdrvctx->config1 = debug_level;
	ret = count;
out:
	mutex_unlock(&mtx);
	return ret;
}

/* Our proc file 1: displays the current value of debug_level */
static int proc_show_debug_level(struct seq_file *seq, void *v)
{
	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;
	seq_printf(seq, "debug_level:%d\n", debug_level);
	mutex_unlock(&mtx);
	return 0;
}

static int myproc_open_dbg_level(struct inode *inode, struct file *file)
{
	return single_open(file, proc_show_debug_level, NULL);
}

static const struct file_operations fops_rdwr_dbg_level = {
	.owner = THIS_MODULE,
	.open = myproc_open_dbg_level,
	.read = seq_read,
	.write = myproc_write_debug_level,
	.llseek = seq_lseek,
	.release = single_release,
};

/*--------------------------------------------------------------------*/
static struct proc_dir_entry *gprocdir;

static int __init procfs_simple_intf_init(void)
{
	int stat = 0;

	if (unlikely(!IS_ENABLED(CONFIG_PROC_FS))) {
		pr_warn("procfs unsupported! Aborting ...\n");
		return -EINVAL;
	}

	/* 0. Create our parent dir under /proc called OURMODNAME */
	gprocdir = proc_mkdir(OURMODNAME, NULL);
	if (!gprocdir) {
		pr_warn("proc_mkdir failed, aborting...\n");
		stat = -ENOMEM;
		goto out_fail_1;
	}
	pr_debug("proc dir (/proc/%s) created\n", OURMODNAME);

	/* 1. Create the PROC_FILE1 proc entry under the parent dir OURMODNAME;
	 * this will serve as the 'dynamically view/modify debug_level'
	 * (pseudo) file. API used:
	 * struct proc_dir_entry *proc_create(const char *name, umode_t mode,
     *             struct proc_dir_entry *parent,
     *             const struct file_operations *proc_fops)
	 */
	if (!proc_create(PROC_FILE1, PROC_FILE1_PERMS, gprocdir,
			&fops_rdwr_dbg_level)) {
		pr_warn("proc_create [1] failed, aborting...\n");
		stat = -ENOMEM;
		goto out_fail_1;
	}
	pr_debug("proc file 1 (/proc/%s/%s) created\n", OURMODNAME, PROC_FILE1);

	/* 2. Create the PROC_FILE2 proc entry under the parent dir OURMODNAME;
	 * this will serve as the 'show PAGE_OFFSET' (pseudo) file;
	 * as it serves precisely one purpose, we use the convenience API
	 * proc_create_single_data() to create this entry. The third parameter
	 * is the function that is called back when this proc entry is read
	 * from userspace. API:
	 * struct proc_dir_entry *proc_create_single_data(const char *name, umode_t mode,
	 *			struct proc_dir_entry *parent,
	 *			int (*show)(struct seq_file *, void *), void *data)
	 */
	if (!proc_create_single_data(PROC_FILE2, PROC_FILE2_PERMS,
					gprocdir, proc_show_pgoff, 0)) {
		pr_warn("proc_create [2] failed, aborting...\n");
		stat = -ENOMEM;
		goto out_fail_2;
	}
	pr_debug("proc file 2 (/proc/%s/%s) created\n", OURMODNAME, PROC_FILE2);

	/* 3. Firstly, allocate and initialize our 'driver context' data structure.
	 * Then create the PROC_FILE3 proc entry under the parent dir OURMODNAME;
	 * this will serve as the 'show driver context' (pseudo) file.
	 * When read from userspace, the callback function will dump the content
	 * of our 'driver context' data structure. API: proc_create()
	 */
	gdrvctx = alloc_init_drvctx();
	if (IS_ERR(gdrvctx)) {
		pr_warn("drv ctx alloc failed, aborting...\n");
		stat = PTR_ERR(gdrvctx);
		goto out_fail_2;
	}
	if (!proc_create(PROC_FILE3, PROC_FILE3_PERMS, gprocdir,
			&fops_show_drvctx)) {
		pr_warn("proc_create [3] failed, aborting...\n");
		stat = -ENOMEM;
		goto out_fail_3;
	}
	pr_debug("proc file 3 (/proc/%s/%s) created\n", OURMODNAME, PROC_FILE3);

	/* 4. Create the PROC_FILE4 proc entry under the parent dir OURMODNAME;
	 * this will serve as the 'read/write drv_ctx->config1' (pseudo) file.
	 * API: proc_create()
	 */
	if (!proc_create(PROC_FILE4, PROC_FILE4_PERMS, gprocdir, &fops_rdwr_config1)) {
		pr_warn("proc_create [4] failed, aborting...\n");
		stat = -ENOMEM;
		goto out_fail_3;
	}
	pr_debug("proc file 4 (/proc/%s/%s) created\n", OURMODNAME, PROC_FILE4);

	pr_info("initialized\n");
	return 0;	/* success */

 out_fail_3:
	kzfree(gdrvctx);
 out_fail_2:
	remove_proc_subtree(OURMODNAME, NULL);
 out_fail_1:
	return stat;
}

static void __exit procfs_simple_intf_cleanup(void)
{
	gdrvctx->power = 0;
	remove_proc_subtree(OURMODNAME, NULL);
	kzfree(gdrvctx);
	pr_info("removed\n");
}

module_init(procfs_simple_intf_init);
module_exit(procfs_simple_intf_cleanup);
