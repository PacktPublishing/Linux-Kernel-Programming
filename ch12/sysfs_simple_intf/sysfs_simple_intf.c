/*
 * ch12/sysfs_simple_intf/sysfs_simple_intf.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 12 : User-Kernel communication pathways
 ****************************************************************
 * Brief Description:
 *
 * Simple demo of having your 2.6 Linux driver create it's own sysfs entry
 * points. For details, pl see Documentation/filesystems/sysfs.txt and the
 * Documentation/driver-model branch.
 *
 * For details, please refer the book, Ch 12.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
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
MODULE_DESCRIPTION
    ("LLKD book:ch12/sysfs_simple_intf: simple sysfs interfacing demo");
/*
 * We *require* the module to be released under GPL license (as well) to please
 * several core driver routines (like sysfs_create_group,
 * platform_device_register_simple, etc which are exported to GPL only (using
 * the EXPORT_SYMBOL_GPL() macro)
 */
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

#define OURMODNAME		"sysfs_simple_intf"
#define SYSFS_FILE1		llkdsysfs_debug_level
#define SYSFS_FILE2		"llkdsysfs_pgoff"

//--- our MSG() macro
#ifdef DEBUG
#define MSG(string, args...)  do {                       \
	pr_info("%s:%s():%d: " string,                   \
		OURMODNAME, __func__, __LINE__, ##args); \
} while (0)
#else
#define MSG(string, args...)
#endif

/* We use a mutex lock; details in Ch 15 and Ch 16 */
DEFINE_MUTEX(mtx);

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
	MSG("In the 'show' method: pressure=%u\n", gpressure);
	n = snprintf(buf, 25, "%u", gpressure);
	mutex_unlock(&mtx);
	return n;
}

/* The DEVICE_ATTR{_RW|RO|WO}() macro instantiates a struct device_attribute
 * dev_attr_<name>
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
	MSG("In the 'show' method: PAGE_OFFSET=0x%lx\n", PAGE_OFFSET);
	n = snprintf(buf, 25, "0x%lx", PAGE_OFFSET);
	mutex_unlock(&mtx);
	return n;
}

/* The DEVICE_ATTR{_RW|RO|WO}() macro instantiates a struct device_attribute
 * dev_attr_<name>
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
	MSG("In the 'show' method: name: %s, debug_level=%d\n", dev->kobj.name,
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
	MSG("In the 'store' method:\ncount=%zu, buf=0x%p count=%zu\n"
	    "Buffer contents: \"%.*s\"\n", count, buf, count, (int)count, buf);
	if (count == 0 || count > 12) {
		ret = -EINVAL;
		goto out;
	}

	ret = kstrtoint(buf, 0, &debug_level);	/* update it! */
	if (ret)
		goto out;
	if (debug_level < DEBUG_LEVEL_MIN || debug_level > DEBUG_LEVEL_MAX) {
		pr_info("%s: trying to set invalid value (%d) for debug_level\n"
			" [allowed range: %d-%d]; resetting to previous (%d)\n",
			OURMODNAME, debug_level, DEBUG_LEVEL_MIN,
			DEBUG_LEVEL_MAX, prev_dbglevel);
		debug_level = prev_dbglevel;
		ret = -EFAULT;
		goto out;
	}
// eg. of using dyn dbg level
// MSG_L1 , MSG_L2 , ...

	ret = count;
 out:
	mutex_unlock(&mtx);
	return ret;
}

/* The DEVICE_ATTR{_RW|RO|WO}() macro instantiates a struct device_attribute
 * dev_attr_<name> (as the comments below help explain ...)
 */
static DEVICE_ATTR_RW(SYSFS_FILE1);	/* it's show/store callbacks are above.. */

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

int __init sysfs_simple_intf_init(void)
{
	int stat = 0;

	/* 0. Register a (dummy) platform device; required as we need a
	 * struct device *dev pointer to create the sysfs file with
	 * the device_create_file() API
	 */
	sysfs_demo_platdev =
	    platform_device_register_simple("sysfs_simple_intf_device", -1,
					    NULL, 0);
	if (IS_ERR(sysfs_demo_platdev)) {
		stat = PTR_ERR(sysfs_demo_platdev);
		pr_info("%s: error (%d) registering our platform device, aborting\n",
			OURMODNAME, stat);
		goto out1;
	}

	// 1. Create our first sysfile file : llkdsysfs_debug_level
	/* The device_create_file() API creates a sysfs attribute file for
	 * given device (1st parameter); the second parameter is the pointer
	 * to it's struct device_attribute structure dev_attr_<name> which was
	 * instantiated by our DEV_ATTR{_RW|RO} macros above ...
	 */
	stat =
	    device_create_file(&sysfs_demo_platdev->dev, &dev_attr_SYSFS_FILE1);
	if (stat) {
		pr_info
		    ("%s: device_create_file [1] failed (%d), aborting now\n",
		     OURMODNAME, stat);
		goto out2;
	}
	MSG("sysfs file [1] (/sys/devices/platform/sysfs_simple_intf_device/%s)"
	    " created\n", "llkdsysfs_debug_level");

	// 2. Create our second sysfile file : llkdsysfs_pgoff
	stat =
	    device_create_file(&sysfs_demo_platdev->dev,
			       &dev_attr_llkdsysfs_pgoff);
	if (stat) {
		pr_info
		    ("%s: device_create_file [2] failed (%d), aborting now\n",
		     OURMODNAME, stat);
		goto out3;
	}
	MSG("sysfs file [2] (/sys/devices/platform/sysfs_simple_intf_device/%s)"
	    " created\n", SYSFS_FILE2);

	// 3. Create our third sysfile file : llkdsysfs_pressure
	gpressure = 25;
	stat =
	    device_create_file(&sysfs_demo_platdev->dev,
			       &dev_attr_llkdsysfs_pressure);
	if (stat) {
		pr_info
		    ("%s: device_create_file [3] failed (%d), aborting now\n",
		     OURMODNAME, stat);
		goto out4;
	}
	MSG("sysfs file [3] (/sys/devices/platform/sysfs_simple_intf_device/%s)"
	    " created\n", "llkdsysfs_pressure");

	pr_info("%s initialized\n", OURMODNAME);
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

void __exit sysfs_simple_intf_cleanup(void)
{
	/* Cleanup sysfs nodes */
	device_remove_file(&sysfs_demo_platdev->dev,
			   &dev_attr_llkdsysfs_pressure);
	device_remove_file(&sysfs_demo_platdev->dev, &dev_attr_llkdsysfs_pgoff);
	device_remove_file(&sysfs_demo_platdev->dev, &dev_attr_SYSFS_FILE1);

	/* Unregister the (dummy) platform device */
	platform_device_unregister(sysfs_demo_platdev);
	pr_info("%s removed\n", OURMODNAME);
}

module_init(sysfs_simple_intf_init);
module_exit(sysfs_simple_intf_cleanup);
