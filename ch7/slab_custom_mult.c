/*
 * ch7/slab_custom_mult/slab_custom_mult.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 7 : Kernel Memory Allocation for Module Authors
 ****************************************************************
 * Brief Description:
 *
 * For details, please refer the book, Ch 7.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/delay.h>

#define OURMODNAME   "slab_custom"
#define OURCACHENAME "our_ctx"

#define ASSERT_HELPER(expr) do {                                       \
 if (!(expr)) {                                                        \
  pr_warn("********** Assertion [%s] failed! : %s:%s:%d **********\n", \
   #expr, __FILE__, __func__, __LINE__);                               \
 }                                                                     \
} while(0)

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LLKD book:ch4/slab_custom: simple demo of creating a custom slab cache");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/* Our 'demo' structure, that we imagine is often allocated and freed;
 * hence, we create a custom slab cache to hold pre-allocated 'instances'
 * of it... It's size: 1360 bytes.
 */
struct myctx {
	u32 iarr[100];
	u64 uarr[100];
	char uname[128], passwd[16], config[16];
};

#define OURMAX_CACHES          200
#define MAX_ALLOW      1*1024*1024

static int xfactor = 300;
module_param(xfactor, int, 0644);
MODULE_PARM_DESC(xfactor, "multiplier factor by which custom cache size increases [def:300]");

static struct kmem_cache *gctx_cachep[OURMAX_CACHES];

static int create_cust_cache(int idx, size_t sz)
{
	int err = 0;

	if (!(idx % 30))
		pr_info("%s:%s():idx %d: required size is %zu bytes\n",
			OURMODNAME, __func__, idx, sz);

	/* Create a new slab cache:
	 * kmem_cache_create(const char *name, unsigned int size, unsigned int align,
                 slab_flags_t flags, void (*ctor)(void *));
	 */
	gctx_cachep[idx] = kmem_cache_create(OURCACHENAME, sz, 0,
			SLAB_POISON | SLAB_RED_ZONE | SLAB_HWCACHE_ALIGN,
			NULL);
	if (!gctx_cachep[idx]) {
		pr_warn("%s:%s():kmem_cache_create() failed\n",
			OURMODNAME, __func__);
		if (IS_ERR(gctx_cachep[idx]))
			err = PTR_ERR(gctx_cachep[idx]);
		return err;
	}
	
	return 0;
}

static void use_our_cache(int idx)
{
	void *obj = NULL;

	if (!(idx % 30)) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39)
		pr_info("Cache name is %s\n", kmem_cache_name(gctx_cachep[idx]));
#endif
		pr_info("%s:%s():idx %d: our cache object size is %u bytes\n",
		OURMODNAME, __func__, idx, kmem_cache_size(gctx_cachep[idx]));
	}

	obj = kmem_cache_alloc(gctx_cachep[idx], GFP_KERNEL);
	if (!obj) {
		pr_warn("%s:%s():kmem_cache_alloc() failed, index %d\n",
			OURMODNAME, __func__, idx);
		return;
	}

	//memset(object, 0xae, sizeof(MyStruct));
	//print_hex_dump_bytes("obj: ", DUMP_PREFIX_OFFSET, obj, 32); //sizeof(struct myctx));
	kmem_cache_free(gctx_cachep[idx], obj);
}

static int create_our_caches(void)
{
	int i;
	size_t sz;

	for (i = 0; i < OURMAX_CACHES; i++) {
		sz = (i+1)*xfactor;
		ASSERT_HELPER(sz > 0 && sz < MAX_ALLOW);
		if (create_cust_cache(i, sz) < 0)
			return -i;
		mdelay(10);
	}
	return 0;
}

static int __init slab_custom_init(void)
{
	int i, ret;

	pr_debug("%s: # custom caches to create = %d, xfactor=%d, MAX_ALLOW=%d\n",
		OURMODNAME, OURMAX_CACHES, xfactor, MAX_ALLOW);
	if ((OURMAX_CACHES*xfactor) >= MAX_ALLOW) {
		pr_info("%s: overflow (xfactor is probably too big)\n", OURMODNAME);
		return -EINVAL;
	}

	if ((ret = create_our_caches()) < 0) {
		pr_warn("%s: custom slab cache creation failed at idx %d! Freeing up...\n",
			OURMODNAME, -1*ret);
		for (i = (-1*ret)-1; i >= 0; i--)
			kmem_cache_destroy(gctx_cachep[i]);
		return -ENOMEM;
	}

	for (i = 0; i < OURMAX_CACHES; i++)
		use_our_cache(i);

	return 0;		/* success */
}

static void __exit slab_custom_exit(void)
{
	int i;

	pr_info("%s: freeing custom caches from 0 to %d ...\n",
		OURMODNAME, OURMAX_CACHES-1);
	for (i = 0; i < OURMAX_CACHES; i++)
		kmem_cache_destroy(gctx_cachep[i]);
	pr_debug("%s: removed\n", OURMODNAME);
}

module_init(slab_custom_init);
module_exit(slab_custom_exit);
