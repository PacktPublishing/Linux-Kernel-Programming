/*
 * user_kernel_comm/sysfs_simple_intf/sysfs_simple_intf.c
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
 *
 * Simple kernel module to demo interfacing with userspace via sysfs.
 * Sysfs is one of several available user<->kernel interfaces; the others
 * include sysfs, debugfs, netlink sockets and the ioctl.
 * In order to demonstrate (and let you easily contrast) between these
 * user<->kernel interfaces, in all cases we create three (or four) 'files' or
 * 'objects' (by issuing the appropriate kernel APIs).
 * In this particular case, the interface is via sysfs, we create three
 * sysfs pseudo-files under a directory whose name is the name given to this
 * kernel module. These three sysfs 'files', what they are named and meant for
 * is summarized below:
 * /sys
 *  ...
 *  |---sysfs_simple_intf            <-- our sysfs directory
 *      |---llkdsysfs_debug_level
 *      |---llkdsysfs_pgoff
 *      |---llkdsysfs_pressure
 *
 * Summary of our sysfs files and how they can be used (R=>read,W=>write)
 * (1) llkdsysfs_debug_level   : RW
 *      R: read retrieves (to userspace) the current value of the global var
 *         debug_level
 *      W: write a value (from userspace) to the global var debug_level, thus
 *         changing the debug level verbosity
 *      file perms: 0644
 * (2) llkdsysfs_pgoff       : R-
 *      R: read retrieves (to userspace) the value of PAGE_OFFSET
 *      file perms: 0444
 * (3) llkdsysfs_pressure    : R-
 *      R: read retrieves (to userspace) the value of the (dummy) global
 *         variable gpressure
 *      file perms: 0440
 *
 * For details, please refer the book.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>

// copy_[to|from]_user()
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 11, 0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LLKD book:user_kernel_comm/sysfs_simple_intf: simple sysfs interfacing demo");
/*
 * We *require* the module to be released under GPL license (as well) to please
 * several core driver routines (like sysfs_create_group,
 * platform_device_register_simple, etc which are exported to GPL only (using
 * the EXPORT_SYMBOL_GPL() macro))
 */
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

#define OURMODNAME		"sysfs_simple_intf"
#define SYSFS_FILE1		llkdsysfs_debug_level
#define SYSFS_FILE2		llkdsysfs_pgoff
#define SYSFS_FILE3		llkdsysfs_pressure

/* We use a mutex lock; details in Ch 15 and Ch 16 */
static DEFINE_MUTEX(mtx);

static int debug_level;		/* 'off' (0) by default ... */
static u32 gpressure;		/* our dummy 'pressure' value */

static struct platform_device *sysfs_demo_platdev;	/* Device structure */

/* Note that in both the show and store methods, the buffer 'buf' is
 * a *kernel*-space buffer. (So don't try copy_[from|to]_user stuff!)
 *
 * From linux/device.h:
--snip--
// interface for exporting device attributes
struct device_attribute {
	struct attribute        attr;
	ssize_t (*show)(struct device *dev, struct device_attribute *attr,
			char *buf);
	ssize_t (*store)(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count);
};
*/

/*------------------ sysfs file 3 (RO) -------------------------------------*/
/* show 'pressure' value: sysfs entry point for the 'show' (read) callback */
static ssize_t llkdsysfs_pressure_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	int n;

	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;
	pr_debug("In the 'show' method: pressure=%u\n", gpressure);
	n = snprintf(buf, 25, "%u", gpressure);
	mutex_unlock(&mtx);
	return n;
}

/* The DEVICE_ATTR{_RW|RO|WO}() macro instantiates a struct device_attribute
 * dev_attr_<name> here...
 * The name of the 'show' callback function is llkdsysfs_pressure_show
 */
static DEVICE_ATTR_RO(llkdsysfs_pressure);

/*------------------ sysfs file 2 (RO) -------------------------------------*/
/* show PAGE_OFFSET value: sysfs entry point for the 'show' (read) callback */
static ssize_t llkdsysfs_pgoff_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	int n;

	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;
	pr_debug("In the 'show' method: PAGE_OFFSET=0x%px\n", (void *)PAGE_OFFSET);
	n = snprintf(buf, 25, "0x%px", (void *)PAGE_OFFSET);
	mutex_unlock(&mtx);
	return n;
}

/* The DEVICE_ATTR{_RW|RO|WO}() macro instantiates a struct device_attribute
 * dev_attr_<name> here...
 * The name of the 'show' callback function is llkdsysfs_pgoff_show
 */
static DEVICE_ATTR_RO(llkdsysfs_pgoff);	/* it's show callback is above.. */

/*------------------ sysfs file 1 (RW) -------------------------------------*/
#define DEBUG_LEVEL_MIN     0
#define DEBUG_LEVEL_MAX     2

/* debug_level: sysfs entry point for the 'show' (read) callback */
static ssize_t llkdsysfs_debug_level_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	int n;

	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;
	pr_debug("In the 'show' method: name: %s, debug_level=%d\n", dev->kobj.name,
	    debug_level);
	n = snprintf(buf, 25, "%d\n", debug_level);
	mutex_unlock(&mtx);
	return n;
}

/* debug_level: sysfs entry point for the 'store' (write) callback */
static ssize_t llkdsysfs_debug_level_store(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	int ret = (int)count, prev_dbglevel;

	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;

	prev_dbglevel = debug_level;
	pr_debug("In the 'store' method:\ncount=%zu, buf=0x%px count=%zu\n"
	    "Buffer contents: \"%.*s\"\n", count, buf, count, (int)count, buf);
	if (count == 0 || count > 12) {
		ret = -EINVAL;
		goto out;
	}

	ret = kstrtoint(buf, 0, &debug_level);	/* update it! */
	if (ret)
		goto out;
	if (debug_level < DEBUG_LEVEL_MIN || debug_level > DEBUG_LEVEL_MAX) {
		pr_info("trying to set invalid value (%d) for debug_level\n"
			" [allowed range: %d-%d]; resetting to previous (%d)\n",
			debug_level, DEBUG_LEVEL_MIN, DEBUG_LEVEL_MAX, prev_dbglevel);
		debug_level = prev_dbglevel;
		ret = -EFAULT;
		goto out;
	}

	ret = count;
 out:
	mutex_unlock(&mtx);
	return ret;
}

/* The DEVICE_ATTR{_RW|RO|WO}() macro instantiates a struct device_attribute
 * dev_attr_<name> (as the comments below help explain ...) here...
 * The name of the 'show' callback function is SYSFS_FILE1_show; similarly,
 * the name of the 'store' callback function is SYSFS_FILE1_store
 */
static DEVICE_ATTR_RW(SYSFS_FILE1);  /* it's show/store callbacks are above */

/*
 * From <linux/device.h>:
DEVICE_ATTR{_RW} helper interfaces (linux/device.h):
--snip--
#define DEVICE_ATTR_RW(_name) \
    struct device_attribute dev_attr_##_name = __ATTR_RW(_name)
#define __ATTR_RW(_name) __ATTR(_name, 0644, _name##_show, _name##_store)
--snip--
and in <linux/sysfs.h>:
#define __ATTR(_name, _mode, _show, _store) {              \
	.attr = {.name = __stringify(_name),               \
		.mode = VERIFY_OCTAL_PERMISSIONS(_mode) }, \
	.show   = _show,                                   \
	.store  = _store,                                  \
}
 */

static int __init sysfs_simple_intf_init(void)
{
	int stat = 0;

	if (unlikely(!IS_ENABLED(CONFIG_SYSFS))) {
		pr_warn("sysfs unsupported! Aborting ...\n");
		return -EINVAL;
	}

	/* 0. Register a (dummy) platform device; required as we need a
	 * struct device *dev pointer to create the sysfs file with
	 * the device_create_file() API:
	 *  struct platform_device *platform_device_register_simple(
	 *		const char *name, int id,
	 *		const struct resource *res, unsigned int num);
	 */
#define PLAT_NAME	"llkd_sysfs_simple_intf_device"
	sysfs_demo_platdev = platform_device_register_simple(PLAT_NAME, -1, NULL, 0);
	if (IS_ERR(sysfs_demo_platdev)) {
		stat = PTR_ERR(sysfs_demo_platdev);
		pr_info("error (%d) registering our platform device, aborting\n", stat);
		goto out1;
	}

	// 1. Create our first sysfile file : llkdsysfs_debug_level
	/* The device_create_file() API creates a sysfs attribute file for
	 * given device (1st parameter); the second parameter is the pointer
	 * to it's struct device_attribute structure dev_attr_<name> which was
	 * instantiated by our DEV_ATTR{_RW|RO} macros above ...
	 * API used:
	 * int device_create_file(struct device *dev,
	 *		const struct device_attribute *attr);
	 */
	stat = device_create_file(&sysfs_demo_platdev->dev, &dev_attr_SYSFS_FILE1);
	    /*
	     * A potentially confusing aspect: as this is the first time, we
	     * explain it via this comment:
	     * The &dev_attr_SYSFS_FILE1 above (2nd param to the
	     * device_create_file() API), is actually *instantiated* via this
	     * declaration above:
	     *  static DEVICE_ATTR_RW(SYSFS_FILE1);
	     * This DEVICE_ATTR{_RW|RO|WO}() macro instantiates a
	     *  struct device_attribute dev_attr_<name>    data structure!
	     * ... and hence we automatically get the rd/wr callbacks registered.
	     * (IOW, the DEVICE_ATTR_XX(name) macro becomes a
	     *  struct device_attribute dev_attr_name data structure!)
	     */
	if (stat) {
		pr_info("device_create_file [1] failed (%d), aborting now\n", stat);
		goto out2;
	}
	pr_debug("sysfs file [1] (/sys/devices/platform/%s/%s) created\n",
		PLAT_NAME, __stringify(SYSFS_FILE1));

	// 2. Create our second sysfile file : llkdsysfs_pgoff
	stat = device_create_file(&sysfs_demo_platdev->dev, &dev_attr_llkdsysfs_pgoff);
	/* As explained above, the
	 *  static DEVICE_ATTR_RO(llkdsysfs_pgoff);
	 * declaration above actually *instantiates* this data structure
	 * (IOW, the DEVICE_ATTR_XX(name) macro becomes a
	 *  struct device_attribute dev_attr_name data structure!)
	 */
	if (stat) {
		pr_info("device_create_file [2] failed (%d), aborting now\n", stat);
		goto out3;
	}
	pr_debug("sysfs file [2] (/sys/devices/platform/%s/%s) created\n",
		PLAT_NAME, __stringify(SYSFS_FILE2));

	// 3. Create our third sysfile file : llkdsysfs_pressure
	gpressure = 25;  // arbitrary 'pressure' value of 25 assigned here..
	stat = device_create_file(&sysfs_demo_platdev->dev, &dev_attr_llkdsysfs_pressure);
	/* As explained above, the
	 *  static DEVICE_ATTR_RO(llkdsysfs_pressure);
	 * declaration above actually *instantiates* this data structure
	 * (IOW, the DEVICE_ATTR_XX(name) macro becomes a
	 *  struct device_attribute dev_attr_name data structure!)
	 */
	if (stat) {
		pr_info("device_create_file [3] failed (%d), aborting now\n", stat);
		goto out4;
	}
	pr_debug("sysfs file [3] (/sys/devices/platform/%s/%s) created\n",
		PLAT_NAME, __stringify(SYSFS_FILE3));

	pr_info("initialized\n");
	return 0;		/* success */
 out4:
	device_remove_file(&sysfs_demo_platdev->dev, &dev_attr_llkdsysfs_pgoff);
 out3:
	device_remove_file(&sysfs_demo_platdev->dev, &dev_attr_SYSFS_FILE1);
 out2:
	platform_device_unregister(sysfs_demo_platdev);
 out1:
	return stat;
}

static void __exit sysfs_simple_intf_cleanup(void)
{
	/* Cleanup sysfs nodes */
	device_remove_file(&sysfs_demo_platdev->dev, &dev_attr_llkdsysfs_pressure);
	device_remove_file(&sysfs_demo_platdev->dev, &dev_attr_llkdsysfs_pgoff);
	device_remove_file(&sysfs_demo_platdev->dev, &dev_attr_SYSFS_FILE1);

	/* Unregister the (dummy) platform device */
	platform_device_unregister(sysfs_demo_platdev);
	pr_info("removed\n");
}

module_init(sysfs_simple_intf_init);
module_exit(sysfs_simple_intf_cleanup);
