/*
 * user_kernel_comm/debugfs_simple_intf/debugfs_simple_intf.c
 **************************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch - User-Kernel communication pathways
 **************************************************************************
 * Brief Description:
 *
 * For details, please refer the book.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#else
#include <asm/uaccess.h>
#endif
#include "../../convenient.h"

#define OURMODNAME      "dbgfs_simple_intf"

MODULE_AUTHOR("<insert name here>");
MODULE_DESCRIPTION(
"LLKD book:ch13/dbgfs_simple_intf: simple demo for U<->K interfacing via debugfs");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/* Module parameters */
static int cause_an_oops;
module_param(cause_an_oops, int, 0644);
MODULE_PARM_DESC(cause_an_oops,
"Setting this to 1 can cause a kernel bug, an Oops; if 1, we do NOT perform"
" required cleanup! so, after removal, any op on the debugfs files will cause"
" an Oops! (default is 0, no bug)");

static struct dentry *gparent;

/* We use a mutex lock; details in Ch 16 and Ch 17 */
DEFINE_MUTEX(mtx);

/* Borrowed from ch11; the 'driver context' data structure;
 * all relevant 'state info' reg the driver and (fictional) 'device'
 * is maintained here.
 */
struct drv_ctx {
	int tx, rx, err, myword, power;
	u32 config1;
	u32 config2;
	u64 config3; /* updated to the 'jiffies' value ... */
#define MAXBYTES   128
	char oursecret[MAXBYTES];
};
static struct drv_ctx *gdrvctx;
static int debug_level;		/* 'off' (0) by default ... */

/* Our debugfs file 1's read callback function */
static ssize_t dbgfs_show_drvctx(struct file *filp, char __user *ubuf,
				 size_t count, loff_t *fpos)
{
	struct drv_ctx *data = (struct drv_ctx *)filp->f_inode->i_private;
			// retrieve the "data" from the inode
#define MAXUPASS 256	// careful- the kernel stack is small!
	char locbuf[MAXUPASS];

	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;

	/* As an experiment, we set our 'config3' member of the drv ctx stucture
	 * to the current 'jiffies' value (# of timer interrupts since boot);
	 * so, every time we 'cat' this file, the 'config3' value should change!
	 */
	data->config3 = jiffies;
	snprintf(locbuf, MAXUPASS - 1,
		 "prodname:%s\n"
		 "tx:%d,rx:%d,err:%d,myword:%d,power:%d\n"
		 "config1:0x%x,config2:0x%x,config3:0x%llx (%llu)\n"
		 "oursecret:%s\n",
		 OURMODNAME,
		 data->tx, data->rx, data->err, data->myword, data->power,
		 data->config1, data->config2, data->config3, data->config3,
		 data->oursecret);

	mutex_unlock(&mtx);
	return simple_read_from_buffer(ubuf, MAXUPASS, fpos, locbuf,
				       strlen(locbuf));
}

static const struct file_operations dbgfs_drvctx_fops = {
	.read = dbgfs_show_drvctx,
};

static struct drv_ctx *alloc_init_drvctx(void)
{
	struct drv_ctx *drvctx = NULL;

	drvctx = kzalloc(sizeof(struct drv_ctx), GFP_KERNEL);
	if (!drvctx)
		return ERR_PTR(-ENOMEM);
	drvctx->config1 = 0x0;
	drvctx->config2 = 0x48524a5f;
	drvctx->config3 = jiffies;
	drvctx->power = 1;
	strncpy(drvctx->oursecret, "AhA yyy", 8);

	pr_info("allocated and init the driver context structure\n");
	return drvctx;
}

static int debugfs_simple_intf_init(void)
{
	int stat = 0;
	struct dentry *file1, *file2;

	if (!IS_ENABLED(CONFIG_DEBUG_FS)) {
		pr_warn("debugfs unsupported! Aborting ...\n");
		return -EINVAL;
	}

	/* 1. Create a dir under the debugfs mount point, whose name is the
	 * module name */
	gparent = debugfs_create_dir(OURMODNAME, NULL);
	if (!gparent) {
		pr_info("debugfs_create_dir failed, aborting...\n");
		stat = PTR_ERR(gparent);
		goto out_fail_1;
	}

	/* 2. Firstly, allocate and initialize our 'driver context' data
	 * structure. Then create the DBGFS_FILE1 entry under the parent dir
	 * OURMODNAME; this will serve as the 'show driver context' (pseudo)
	 * file. When read from userspace, the callback function will dump the
	 * content of our 'driver context' data structure.
	 */
	gdrvctx = alloc_init_drvctx();
	if (IS_ERR(gdrvctx)) {
		pr_info("drv ctx alloc failed, aborting...\n");
		stat = PTR_ERR(gdrvctx);
		goto out_fail_2;
	}

	/* Generic debugfs file + passing a pointer to a data structure as a
	 * demo.. the 4th param is a generic void * ptr; it's contents will be
	 * stored into the i_private field of the file's inode.
	 */
#define DBGFS_FILE1	"llkd_dbgfs_show_drvctx"
	file1 =
	    debugfs_create_file(DBGFS_FILE1, 0440, gparent, (void *)gdrvctx,
				&dbgfs_drvctx_fops);
	if (!file1) {
		pr_info("debugfs_create_file failed, aborting...\n");
		stat = PTR_ERR(file1);
		goto out_fail_3;
	}
	pr_debug("debugfs file 1 <debugfs_mountpt>/%s/%s created\n",
		 OURMODNAME, DBGFS_FILE1);

	/* 3. Create the debugfs file for the debug_level global; we use the
	 * helper routine to make it simple! There is a downside: we have no
	 * chance to perform a validity check on the value being written..
	 */
#define DBGFS_FILE2	"llkd_dbgfs_debug_level"
	file2 = debugfs_create_u32(DBGFS_FILE2, 0644, gparent, &debug_level);
	if (!file2) {
		pr_info("debugfs_create_u32 failed, aborting...\n");
		stat = PTR_ERR(file2);
		goto out_fail_3;
	}
	pr_debug("debugfs file 2 <debugfs_mountpt>/%s/%s created\n", OURMODNAME, DBGFS_FILE2);

	pr_info("initialized (fyi, our 'cause an Oops' setting is currently %s)\n",
		cause_an_oops == 1 ? "On" : "Off");
	return 0;

 out_fail_3:
	kfree(gdrvctx);
 out_fail_2:
	debugfs_remove_recursive(gparent);
 out_fail_1:
	return stat;
}

static void debugfs_simple_intf_cleanup(void)
{
	kfree(gdrvctx);
	if (!cause_an_oops)
		debugfs_remove_recursive(gparent);
	pr_info("removed\n");
}

module_init(debugfs_simple_intf_init);
module_exit(debugfs_simple_intf_cleanup);
