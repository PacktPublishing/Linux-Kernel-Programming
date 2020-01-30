/**
 * procfs_simple_intf.c
 *
 * Simple kernel module to demo creation and usage of procfs entries.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

// copy_[to|from]_user()
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,11,0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

#define	OURMODNAME		"procfs_simple_intf"
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

/* Display the system PAGE_OFFSET value */
static int myproc_show_pgoff(struct seq_file *seq, void *v)
{
	MSG("\n");
	seq_printf(seq, "%s:PAGE_OFFSET:0x%lx\n", OURMODNAME, PAGE_OFFSET);
	return 0;
}

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

static int myproc_open_pgoff(struct inode *inode, struct file *file)
{
	return single_open(file, myproc_show_pgoff, NULL);
}

static const struct file_operations myproc_fops_pgoff = {
	.owner = THIS_MODULE,
	.open = myproc_open_pgoff,
	.read = seq_read,
	//.write = myproc_write,
	//.llseek = seq_lseek,
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
	struct proc_dir_entry *procent_pgoff;

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

	gprocdir = proc_mkdir(OURMODNAME, NULL);
	if (!gprocdir) {
		pr_warn("%s: proc_mkdir failed, aborting...\n", OURMODNAME);
		stat = -ENOMEM;
		goto out_fail_2;
	}
	procent_pgoff = proc_create("show_pgoff", 0644, gprocdir, &myproc_fops_pgoff);
	if (!procent_pgoff) {
		pr_warn("%s: proc_create (show_pgoff) failed, aborting...\n", OURMODNAME);
		stat = -ENOMEM;
		goto out_fail_3;
	}
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
