/*
 * solutions_to_assgn/user_kernel_comm/sysfs_addrxlate/sysfs_addrxlate.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 13 : User-Kernel communication pathways
 ****************************************************************
 * Brief Description:
 * This is an assignment from the chapter "User-Kernel communication pathways".
 *
 * sysfs_addrxlate: sysfs assignment #2 (a bit more advanced):
 * Address translation: exploiting the knowledge gained from this chapter and
 * Ch 7 section 'Direct-mapped RAM and address translation', write a simple
 * platform driver that provides two sysfs interface files called
 * addrxlate_kva2pa and addrxlate_pa2kva; the way it should work: writing a
 * kva (kernel virtual address) into the sysfs file addrxlate_kva2pa should
 * have the driver read and translate the kva into it's corresponding physical
 * address (pa); then reading from the same file should cause the pa to be
 * displayed. Vice-versa with the addrxlate_pa2kva sysfs file.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/mm.h>	    // for high_memory
#include <asm/io.h>			// virt_to_phys() / phys_to_virt()

// copy_[to|from]_user()
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 11, 0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION
("LLKD book:solutions_to_assgn/user_kernel_comm/sysfs_addrxlate: simple sysfs interfacing to translate linear addr");
/*
 * We *require* the module to be released under GPL license (as well) to please
 * several core driver routines (like sysfs_create_group,
 * platform_device_register_simple, etc which are exported to GPL only (using
 * the EXPORT_SYMBOL_GPL() macro)
 */
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

#define OURMODNAME	"sysfs_addrxlate"
#define SYSFS_FILE1	addrxlate_kva2pa
#define SYSFS_FILE2	addrxlate_pa2kva

/* We use a mutex lock; details in Ch 16 and Ch 17 */
static DEFINE_MUTEX(mtx1);
static DEFINE_MUTEX(mtx2);
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

#define MANUALLY
#define ADDR_MAXLEN	20
static phys_addr_t gxlated_addr_kva2pa;
//static unsigned long gxlated_addr_pa2kva;
static size_t gxlated_addr_pa2kva; // ISSUE on x86_64 ??

/*------------------ sysfs file 2 (RW) -------------------------------------*/

/* xlateaddr_pa2kva: sysfs entry point for the 'show' (read) callback */
static ssize_t addrxlate_pa2kva_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	int n;

	if (mutex_lock_interruptible(&mtx2))
		return -ERESTARTSYS;
	pr_debug("In the 'show' method\n");
	n = snprintf(buf, ADDR_MAXLEN, "0x%px\n", (void *)gxlated_addr_pa2kva);
	mutex_unlock(&mtx2);

	return n;
}

/* xlateaddr_pa2kva: sysfs entry point for the 'store' (write) callback */
static ssize_t addrxlate_pa2kva_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	int ret = (int)count;
	char s_addr[ADDR_MAXLEN];
	phys_addr_t pa = 0x0;

	if (mutex_lock_interruptible(&mtx2))
		return -ERESTARTSYS;

	memset(s_addr, 0, ADDR_MAXLEN);
	strncpy(s_addr, buf, ADDR_MAXLEN);
	s_addr[strnlen(s_addr, ADDR_MAXLEN) - 1] = '\0';	// rm trailing newline char
	if (count == 0 || count > ADDR_MAXLEN) {
		ret = -EINVAL;
		goto out;
	}
	//ret = kstrtoul(s_addr, 0, (long unsigned int *)&pa);
#if(BITS_PER_LONG == 32)
	ret = kstrtoul(s_addr, 0, (long unsigned int *)&pa);
#else
	ret = kstrtoull(s_addr, 0, &pa);
#endif
	if (ret < 0) {
		mutex_unlock(&mtx2);
		pr_warn("kstrtoul failed!\n");
		return ret;
	}

	/* Verify that the passed pa is valid
	 * WARNING! the below validity checks are very simplistic; YMMV!
	 */
	if (pa > PAGE_OFFSET) {
		mutex_unlock(&mtx1);
		pr_info("invalid physical address (0x%pa)?\n",&pa);
		return -EFAULT;
	}

	/* All okay (fingers crossed), perform the address translation! */
	gxlated_addr_pa2kva = (size_t)phys_to_virt(pa);
	pr_debug(" pa 0x%pa = KVA 0x%px\n", &pa, (void *)gxlated_addr_pa2kva);

#ifdef MANUALLY
	/* 'Manually' perform the address translation */
	pr_info("manually:  pa 0x%pa = KVA 0x%px\n",
		&pa, (void *)(pa + PAGE_OFFSET));
#endif
	ret = count;
 out:
	mutex_unlock(&mtx2);
	return ret;
}

static DEVICE_ATTR_RW(SYSFS_FILE2);	/* it's show/store callbacks are above */

/*------------------ sysfs file 1 (RW) -------------------------------------*/

/* xlateaddr_kva2pa: sysfs entry point for the 'show' (read) callback */
static ssize_t addrxlate_kva2pa_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	int n;

	if (mutex_lock_interruptible(&mtx1))
		return -ERESTARTSYS;
	pr_debug("In the 'show' method\n");
	n = snprintf(buf, ADDR_MAXLEN, "0x%px\n", (void *)gxlated_addr_kva2pa);
	//n = snprintf(buf, ADDR_MAXLEN, "0x%llx\n", gxlated_addr_kva2pa);
	mutex_unlock(&mtx1);

	return n;
}

/* xlateaddr_kva2pa: sysfs entry point for the 'store' (write) callback */
static ssize_t addrxlate_kva2pa_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	int ret = (int)count, valid = 1;
	char s_addr[ADDR_MAXLEN];
	size_t kva = 0x0;
	//unsigned long kva = 0x0;

	if (mutex_lock_interruptible(&mtx1))
		return -ERESTARTSYS;

	memset(s_addr, 0, ADDR_MAXLEN);
	strncpy(s_addr, buf, ADDR_MAXLEN);
	//pr_info("s_addr = %s, count=%ld\n", s_addr, count);
	s_addr[strnlen(s_addr, ADDR_MAXLEN) - 1] = '\0';	// rm trailing newline char
	if (count == 0 || count > ADDR_MAXLEN) {
		pr_info("invalid addr length (count=%ld)\n", count);
		ret = -EINVAL;
		goto out;
	}
	//ret = kstrtoul(s_addr, 0, (long unsigned int *)&kva);
#if(BITS_PER_LONG == 32)
	ret = kstrtoul(s_addr, 0, (long unsigned int *)&kva);
#else
	ret = kstrtoull(s_addr, 0, &kva);
#endif
	if (ret < 0) {
		mutex_unlock(&mtx1);
		pr_warn("kstrtoul failed!\n");
		return ret;
	}

	/* Verify that the passed kva is valid, a linear address, i.e, it
	 * lies within the lowmem segment, i.e., between PAGE_OFFSET and
	 * high_memory; x86 has a builtin validity checker func that we use,
	 * for other we do our own (simplistic) checking.
	 */
#ifdef CONFIG_X86
	if (!virt_addr_valid(kva))
		valid = 0;
#else
	// WARNING! the below validity checks are very simplistic; YMMV!
	if ((kva < PAGE_OFFSET) || (kva > (size_t)high_memory))
		valid = 0;
#endif
	if (!valid) {
		mutex_unlock(&mtx1);
		pr_info("invalid virtual address (0x%px),"
		" must be a valid linear addr within the kernel lowmem region\n"
		" IOW, *only* kernel direct mapped RAM locations are valid\n",
			(void *)kva);
		return -EFAULT;
	}

	/* All okay (fingers crossed), perform the address translation! */
	gxlated_addr_kva2pa = virt_to_phys((volatile void *)kva);
	pr_debug("KVA 0x%px =  pa 0x%pa\n", (void *)kva, &gxlated_addr_kva2pa);

#ifdef MANUALLY
	/* 'Manually' perform the address translation */
	pr_info("%s: manually: KVA 0x%px =  pa 0x%px\n",
		OURMODNAME, (void *)kva, (void *)(kva - PAGE_OFFSET));
#endif
	ret = count;
 out:
	mutex_unlock(&mtx1);
	return ret;
}

/* The DEVICE_ATTR{_RW|RO|WO}() macro instantiates a struct device_attribute
 * dev_attr_<name> (as the comments below help explain ...) here...
 */
static DEVICE_ATTR_RW(SYSFS_FILE1);	/* it's show/store callbacks are above */

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

static int __init sysfs_addrxlate_init(void)
{
	int stat = 0;

	if (!IS_ENABLED(CONFIG_SYSFS)) {
		pr_warn("sysfs unsupported! Aborting ...\n");
		return -EINVAL;
	}

	/* 0. Register a (dummy) platform device; required as we need a
	 * struct device *dev pointer to create the sysfs file with
	 * the device_create_file() API
	 */
#define PLAT_NAME	"llkd_sysfs_addrxlate"
	sysfs_demo_platdev =
	    platform_device_register_simple(PLAT_NAME, -1, NULL, 0);
	if (IS_ERR(sysfs_demo_platdev)) {
		stat = PTR_ERR(sysfs_demo_platdev);
		pr_info("error (%d) registering our platform device, aborting\n", stat);
		goto out1;
	}

	// 1. Create our first sysfile file : addrxlate_kva2pa
	/* The device_create_file() API creates a sysfs attribute file for
	 * given device (1st parameter); the second parameter is the pointer
	 * to it's struct device_attribute structure dev_attr_<name> which was
	 * instantiated by our DEV_ATTR{_RW|RO} macros above ...
	 */
	stat =
	    device_create_file(&sysfs_demo_platdev->dev, &dev_attr_SYSFS_FILE1);
	if (stat) {
		pr_info ("device_create_file [1] failed (%d), aborting now\n", stat);
		goto out2;
	}
	pr_info("sysfs file [1] (/sys/devices/platform/%s/%s) created\n",
	    PLAT_NAME, __stringify(SYSFS_FILE1));

	// 2. Create our second sysfile file : addrxlate_pa2kva
	stat =
	    device_create_file(&sysfs_demo_platdev->dev, &dev_attr_SYSFS_FILE2);
	if (stat) {
		pr_info ("device_create_file [2] failed (%d), aborting now\n", stat);
		goto out3;
	}
	pr_info("sysfs file [2] (/sys/devices/platform/%s/%s) created\n",
	    PLAT_NAME, __stringify(SYSFS_FILE2));

	pr_info("initialized\n");
	return 0;		/* success */

 out3:
	device_remove_file(&sysfs_demo_platdev->dev, &dev_attr_SYSFS_FILE1);
 out2:
	platform_device_unregister(sysfs_demo_platdev);
 out1:
	return stat;
}

static void __exit sysfs_addrxlate_cleanup(void)
{
	/* Cleanup sysfs nodes */
	device_remove_file(&sysfs_demo_platdev->dev, &dev_attr_SYSFS_FILE2);
	device_remove_file(&sysfs_demo_platdev->dev, &dev_attr_SYSFS_FILE1);
	/* Unregister the (dummy) platform device */
	platform_device_unregister(sysfs_demo_platdev);
	pr_info("removed\n");
}

module_init(sysfs_addrxlate_init);
module_exit(sysfs_addrxlate_cleanup);
