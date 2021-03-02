/*
 * solutions_to_assgn/ch9/slab_custom_mult/slab_custom_mult.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming
 *
 * From: Ch 9 : Kernel Memory Allocation for Module Authors Part 2
 ****************************************************************
 * Brief Description:
 * Assignment:
 * 
 * Write a kernel module that creates multiple custom slab caches. Fix the total
 * number of custom caches as, say, 100. Make the name of the caches as
 * our_slab-n; where 'n' is an integer from 0 to 99.
 * The *size* of the object in each cache also varies; start with a fixed size,
 * say, 300 bytes, and keep incrementing the size by a 'multiplier factor' (say,
 * 300 bytes). So, it looks like this:
 * 
 * Slab cache Name     Size
 *  our_slab-0          300
 *  our_slab-2          600
 *  our_slab-3          900
 *          [...]
 *  our_slab-98       29700
 *  our_slab-99       30000
 * 
 * Why not make the 'multiplier size' a module parameter, called 'xfactor'.
 * After it's loaded, lookup the kernel log and use vmstat(8) (or other utils)
 * to see all our custom slab caches! Also do a 
 *  sudo vmstat -m |grep "^our_slab"
 * to see how much memory is *actually* allocated to each object in each custom
 * slab cache (it's the fourth column, as we saw earlier).
 * 
 * For details, please refer the book, Ch 9.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>

#define OURMODNAME   "slab_custom_mult"
#define OURCACHENAME "our_slab"

#define ASSERT_HELPER(expr) do {                                       \
 if (!(expr)) {                                                        \
  pr_warn("********** Assertion [%s] failed! : %s:%s:%d **********\n", \
   #expr, __FILE__, __func__, __LINE__);                               \
 }                                                                     \
} while(0)

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKDC book:solutions_to_assgn/ch9/slab_custom_mult: assigment solution");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

#define OURMAX_CACHES  100
#define MAX_ALLOW   1*1024*1024

static int xfactor = 300;
module_param(xfactor, int, 0644);
MODULE_PARM_DESC(xfactor, "multiplier factor by which custom cache size increases [def=300]");

/* An array of pointers to our custom 'llkd' slab caches */
static struct kmem_cache *our_llkd_cachep[OURMAX_CACHES];

static void use_our_cache(int idx)
{
	void *obj = NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39)
	pr_info("Cache name is %s\n", kmem_cache_name(our_llkd_cachep[idx]));
#endif
	pr_info("idx %d: our cache object size is %u bytes\n",
		idx, kmem_cache_size(our_llkd_cachep[idx]));

	obj = kmem_cache_alloc(our_llkd_cachep[idx], GFP_KERNEL);
	if (!obj) {
		pr_warn("kmem_cache_alloc() failed, index %d\n", idx);
		return;
	}

	kmem_cache_free(our_llkd_cachep[idx], obj);
}

static int create_cust_llkd_cache(int idx, size_t sz)
{
	int err = 0;
	char nm[128];

	if (!(idx % 30))
		pr_info("idx %d: required size is %zu bytes\n", idx, sz);

	/* Create a new slab cache:
	 * kmem_cache_create(const char *name, unsigned int size, unsigned int align,
                 slab_flags_t flags, void (*ctor)(void *));
	 */
	memset(nm, 0, 128);
	snprintf(nm, 127, "%s-%d", OURCACHENAME, idx);
	our_llkd_cachep[idx] = kmem_cache_create(nm, sz, 0,
			SLAB_POISON | SLAB_RED_ZONE | SLAB_HWCACHE_ALIGN,
			NULL);
	if (!our_llkd_cachep[idx]) {
		pr_warn("kmem_cache_create() on index %d failed\n", idx);
		if (IS_ERR(our_llkd_cachep[idx]))
			err = PTR_ERR(our_llkd_cachep[idx]);
		return err;
	}
	
	return 0;
}

static int create_our_llkd_caches(void)
{
	int i;
	size_t sz;

	for (i = 0; i < OURMAX_CACHES; i++) {
		sz = (i+1)*xfactor;
		ASSERT_HELPER(sz > 0 && sz < MAX_ALLOW);
		if (create_cust_llkd_cache(i, sz) < 0)
			return -i;
	}
	return 0;
}

static int __init slab_custom_mult_init(void)
{
	int i, ret;

	pr_info("# custom caches to create = %d, xfactor=%d, MAX_ALLOW=%d\n",
		OURMAX_CACHES, xfactor, MAX_ALLOW);
	if ((OURMAX_CACHES*xfactor) >= MAX_ALLOW) {
		pr_info("overflow (xfactor is probably too big)\n");
		return -EINVAL;
	}

	if ((ret = create_our_llkd_caches()) < 0) {
		pr_warn("custom slab cache creation failed at idx %d! Freeing up...\n",
			-1*ret);
		for (i = (-1*ret)-1; i >= 0; i--)
			kmem_cache_destroy(our_llkd_cachep[i]);
		return -ENOMEM;
	}

	for (i = 0; i < OURMAX_CACHES; i++)
		use_our_cache(i);

	return 0;		/* success */
}

static void __exit slab_custom_mult_exit(void)
{
	int i;

	pr_info("freeing custom caches from 0 to %d ...\n", OURMAX_CACHES-1);
	for (i = 0; i < OURMAX_CACHES; i++)
		kmem_cache_destroy(our_llkd_cachep[i]);
	pr_info("removed\n");
}

module_init(slab_custom_mult_init);
module_exit(slab_custom_mult_exit);
