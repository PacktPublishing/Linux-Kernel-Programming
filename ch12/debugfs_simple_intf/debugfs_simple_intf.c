/*
 * ch12/debugfs_simple_intf/debugfs_simple_intf.c
 **************************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 12 : User-Kernel communication pathways
 **************************************************************************
 * Brief Description:
 *
 * For details, please refer the book, Ch 12.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include "../../convenient.h"

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#else
#include <asm/uaccess.h>
#endif

#define OURMODNAME      "dbgfs_simple_intf"

MODULE_AUTHOR("<insert name here>");
MODULE_DESCRIPTION(
 "LLKD book:ch12/dbgfs_simple_intf: simple demo for U<->K interfacing via debugfs");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static struct dentry *gparent;

/* We use a mutex lock; details in Ch 15 and Ch 16 */
DEFINE_MUTEX(mtx);

/* Borrowed from ch11; the 'driver context' data structure;
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


#if 0
//- 1. -------------------------------------------Generic R/W debugfs hooks
static ssize_t dbgfs_genread(struct file *filp, char __user *ubuf, size_t count, loff_t *fpos)
{
	char *data =  (char *)filp->f_inode->i_private; // retrieve the "data" from the inode
	//char *data =  (char *)filp->f_dentry->d_inode->i_private; // [OLD] retrieve the "data" from the inode
	MSG("data: %s len=%ld\n", data, strlen(data));

	/* simple_read_from_buffer - copy data from the buffer to user space:
     * @to: the user space buffer to read to
     * @count: the maximum number of bytes to read
     * @ppos: the current position in the buffer
     * @from: the buffer to read from
     * @available: the size of the buffer
     *
     * The simple_read_from_buffer() function reads up to @count bytes from the
     * buffer @from at offset @ppos into the user space address starting at @to.
     *
     * On success, the number of bytes read is returned and the offset @ppos is
     * advanced by this number, or negative value is returned on error.

         ssize_t simple_read_from_buffer(void __user *to, size_t count, loff_t *ppos,
                 const void *from, size_t available)
	*/
	return simple_read_from_buffer(ubuf, strlen(data), fpos, data, strlen(data));
}

static ssize_t dbgfs_genwrite(struct file *filp, const char __user *ubuf, size_t count, loff_t *fpos)
{
	char udata[MAXUPASS];
	QP;
	if (count > MAXUPASS) {
		printk("%s: too much data attempted to be passed from userspace to here: %s:%d\n", 
			OURMODNAME, __FUNCTION__, __LINE__);
		return -ENOSPC;
	}
	if (copy_from_user(udata, ubuf, count)) {
		printk("%s:%s:%d: copy_from_user failed!\n", 
			OURMODNAME, __FUNCTION__, __LINE__);
		return -EIO;
	}
	print_hex_dump_bytes(" ", DUMP_PREFIX_NONE, udata, count);
	return count;
}


//- 2. -----------------------------------------------------Passing a structure pointer
typedef struct {
	short tx, rx;
	u32 j;
	char sec[20];
} MYS;
static MYS *mine;

static ssize_t dbgfs_genread2(struct file *filp, char __user *ubuf, size_t count, loff_t *fpos)
{
	MYS *data =  (MYS *)filp->f_inode->i_private; // retrieve the "data" from the inode
	//MYS *data =  (MYS *)filp->f_dentry->d_inode->i_private; // retrieve the "data" from the inode
	char loc[MAXUPASS];

	data->j = jiffies;
	MSG("data: tx=%d rx=%d j=%u sec=%s\n", data->tx, data->rx, data->j, data->sec);
	snprintf(loc, count-1, "data: tx=%d rx=%d j=%u sec=%s\n", data->tx, data->rx, data->j, data->sec);
	return simple_read_from_buffer(ubuf, MAXUPASS, fpos, loc, strlen(loc));
}

static struct file_operations dbg_fops2 = {
	.read = dbgfs_genread2,
};

//- 3. -------------------------------------------- Various debugfs helpers
static u32 myu32=100;
static u64 myu64=64;
static u32 myx32=0x100;
static bool mybool=1;

// 5. -------------------------------------------------------- Blob wrapper
static struct debugfs_blob_wrapper myblob;


//-----------------------------------------------------------------------------
static int setup_debugfs_entries(void)
{
	QP;

	/* 1. Generic debugfs file + passing a pointer to string as a demo..
       4th param is a generic void * ptr; it's contents will be stored into the i_private field
	   of the file's inode.
	*/
	if (!debugfs_create_file("generic_1", 0644, gparent, (void *)"somejunk data", &dbg_fops1)) {
		DBGFS_CREATE_ERR(gparent, "debugfs_create_file 1");
	}

	/* 2. Generic debugfs file + passing a pointer to a data structure as a demo..
       4th param is a generic void * ptr; it's contents will be stored into the i_private field
	   of the file's inode.
	*/
	mine = (MYS *)kmalloc (sizeof(MYS), GFP_KERNEL);
	if (!mine) {
		printk(KERN_ALERT "%s: kmalloc failed!\n", OURMODNAME);
		return -ENOMEM;
	}
	mine->tx = mine->rx = 0;
	mine->j = jiffies;
	strncpy (mine->sec, "Security Proto 1B", 20);
	if (!debugfs_create_file("generic_2", 0440, gparent, (void *)mine, &dbg_fops2)) {
		DBGFS_CREATE_ERR(gparent, "debugfs_create_file 2");
	}

	/* 3. In a number of cases, the creation of a set of file operations is not
       actually necessary; the debugfs code provides a number of helper functions
       for simple situations.  ...
      struct dentry *debugfs_create_u32(const char *name, mode_t mode,
				      struct dentry *parent, u32 *value); 
	  ...
    */
	if (!debugfs_create_u32("helper_u32", 0644, gparent, &myu32)) {
		DBGFS_CREATE_ERR(gparent, "debugfs_create_u32 ");
	}
	if (!debugfs_create_u64("helper_u64", 0644, gparent, &myu64)) {
		DBGFS_CREATE_ERR(gparent, "debugfs_create_u64 ");
	}
	// For hex, use the debugfs_create_x[8|16|32|64] helpers...
	if (!debugfs_create_x32("helper_x32", 0644, gparent, &myx32)) {
		DBGFS_CREATE_ERR(gparent, "debugfs_create_x32 ");
	}

	/* 4. Boolean values can be placed in debugfs with:

       struct dentry *debugfs_create_bool(const char *name, mode_t mode,
				       struct dentry *parent, u32 *value);

       A read on the resulting file will yield either Y (for non-zero values) or
       N, followed by a newline.  If written to, it will accept either upper- or
       lower-case values, or 1 or 0.  Any other input will be silently ignored.
     */
	if (!debugfs_create_bool("helper_bool", 0644, gparent, &mybool)) {
		DBGFS_CREATE_ERR(gparent, "debugfs_create_bool ");
	}

	/* 5. A block of arbitrary binary data can be exported with:

           struct debugfs_blob_wrapper {
           	   void *data;
       	       unsigned long size;
           };

           struct dentry *debugfs_create_blob(const char *name, mode_t mode,
				       struct dentry *parent,
				       struct debugfs_blob_wrapper *blob);

       A read of this file will return the data pointed to by the
       debugfs_blob_wrapper structure.  Some drivers use "blobs" as a simple way
       to return several lines of (static) formatted text output.  This function
       can be used to export binary information, but there does not appear to be
       any code which does so in the mainline.  Note that all files created with
       debugfs_create_blob() are read-only.
	*/
	myblob.data = mine;
	myblob.size = sizeof(MYS);
	if (!debugfs_create_blob("myblob", 0444, gparent, &myblob)) {
		DBGFS_CREATE_ERR(gparent, "debugfs_create_blob ");
	}

	// 6. Soft link
	if  (!debugfs_create_symlink("thedata", gparent, "generic_2")) {
		DBGFS_CREATE_ERR(gparent, "debugfs_create_symlink ");
	}
	return 0;
}
#endif

#define MAXUPASS 256 // careful- k stack is small!

static ssize_t dbgfs_show_drvctx(struct file *filp, char __user *ubuf, size_t count, loff_t *fpos)
{
        struct drv_ctx *data =  (struct drv_ctx *)filp->f_inode->i_private; // retrieve the "data" from the inode
        //MYS *data =  (MYS *)filp->f_dentry->d_inode->i_private; // retrieve the "data" from the inode
        char locbuf[MAXUPASS];

	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;

	/* As an experiment, we set our 'config3' member of the drv ctx stucture
	 * to the current 'jiffies' value (# of timer interrupts since boot!);
	 * so, every time we 'cat' this file, the 'config3' value should change!
	 */
	data->config3 = jiffies;
	snprintf(locbuf, MAXUPASS-1,
		"prodname:%s\n"
		"tx:%d,rx:%d,err:%d,myword:%d,power:%d\n"
		"config1:0x%x,config2:0x%x,config3:0x%llx (%llu)\n"
		"oursecret:%s\n",
		OURMODNAME,
		data->tx, data->rx, data->err, data->myword, data->power,
		data->config1, data->config2, data->config3, data->config3,
		data->oursecret);

	mutex_unlock(&mtx);
        return simple_read_from_buffer(ubuf, MAXUPASS, fpos, locbuf, strlen(locbuf));
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
	drvctx->config3 = 0x424c0a52;
	drvctx->power = 1;
	strncpy(drvctx->oursecret, "AhA yyy", 8);

	pr_debug("%s: allocated and init the driver context structure\n",
		OURMODNAME);
	return drvctx;
}

static int debugfs_simple_intf_init(void)
{
	int stat = 0;
	struct dentry *file1;

	if (!IS_ENABLED(CONFIG_DEBUG_FS)) {
		pr_warn("%s: debugfs unsupported! Aborting ...\n", OURMODNAME);
		return -EINVAL;
	}

	/* 1. Create a dir under the debugfs mount point, whose name is the
	 * module name */
	gparent = debugfs_create_dir(OURMODNAME, NULL);
	if (!gparent) {
		pr_info("%s: debugfs_create_dir failed, aborting...\n", OURMODNAME);
		stat = PTR_ERR(gparent);
		goto out_fail_1;
	}

	/* 2. Firstly, allocate and initialize our 'driver context' data structure.
	 * Then create the DBGFS_FILE1 entry under the parent dir OURMODNAME;
	 * this will serve as the 'show driver context' (pseudo) file.
	 * When read from userspace, the callback function will dump the content
	 * of our 'driver context' data structure.
	 */
	gdrvctx = alloc_init_drvctx();
	if (IS_ERR(gdrvctx)) {
		pr_info("%s: drv ctx alloc failed, aborting...\n", OURMODNAME);
		stat = PTR_ERR(gdrvctx);
		goto out_fail_2;
	}

	/* Generic debugfs file + passing a pointer to a data structure as a demo..
       4th param is a generic void * ptr; it's contents will be stored into the i_private field
	   of the file's inode.
	*/
#define DBGFS_FILE1	"llkd_show_drvctx"
	file1 = debugfs_create_file(DBGFS_FILE1, 0440, gparent, (void *)gdrvctx, &dbgfs_drvctx_fops);
	if (!file1) {
		pr_info("%s: debugfs_create_file failed, aborting...\n", OURMODNAME);
		stat = PTR_ERR(file1);
		goto out_fail_3;
	}
	pr_debug("%s: debugfs file 1 (<debugfs_mountpt>/%s/%s) created\n",
		OURMODNAME, OURMODNAME, DBGFS_FILE1);

	pr_info("%s initialized\n", OURMODNAME);
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
	debugfs_remove_recursive(gparent);
	pr_info("%s removed\n", OURMODNAME);
}

module_init(debugfs_simple_intf_init);
module_exit(debugfs_simple_intf_cleanup);
