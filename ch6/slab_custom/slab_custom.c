/*
 * ch6/slab_custom/slab_custom.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch 6 : Kernel Memory Allocation for Module Authors Part 2
 ****************************************************************
 * Brief Description:
 * Simple demo of using the slab layer (exorted) APIs to create our very own
 * custom slab cache.
 *
 * For details, please refer the book, Ch 6.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/sched.h>   /* current */

#define OURMODNAME   "slab_custom"
#define OURCACHENAME "our_ctx"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKDC book:ch6/slab_custom: simple demo of creating a custom slab cache");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/* Our 'demo' structure, that we imagine is often allocated and freed;
 * hence, we create a custom slab cache to hold pre-allocated 'instances'
 * of it... It's size: 328 bytes.
 */
struct myctx {
	u32 iarr[10];
	u64 uarr[10];
	char uname[128], passwd[16], config[64];
};
static struct kmem_cache *gctx_cachep;

static void use_our_cache(void)
{
	struct myctx *obj = NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39)
	pr_info("Cache name is %s\n", kmem_cache_name(gctx_cachep));
#else
	pr_info("[ker ver > 2.6.38 cache name deprecated...]\n");
#endif

	obj = kmem_cache_alloc(gctx_cachep, GFP_KERNEL);
	if (!obj) {
		pr_warn("%s:%s():kmem_cache_alloc() failed\n",
			OURMODNAME, __func__);
	}

	pr_info("Our cache object (@ %pK, actual=%llx) size is %u bytes; ksize=%lu\n",
		obj, (long long unsigned)obj, kmem_cache_size(gctx_cachep), ksize(obj));
	print_hex_dump_bytes("obj: ", DUMP_PREFIX_OFFSET, obj, sizeof(struct myctx));

	kmem_cache_free(gctx_cachep, obj);
}

/* The parameter is the pointer to the just allocated memory 'object' from
 * our custom slab cache; here, this is our 'constructor' routine; so, we
 * initialize our just allocated memory object.
 */
static void our_ctor(void *foo)
{
	struct myctx *ctx = foo;
	struct task_struct *p = current;

	memset(ctx, 0, sizeof(struct myctx));
	/* As a demo, we init the 'config' field of our structure to some
	 * (arbitrary) 'accounting' values from our task_struct
	 */
	snprintf(ctx->config, 6*sizeof(u64)+5, "%d.%d,%ld.%ld,%ld,%ld",
		p->tgid, p->pid,
		p->nvcsw, p->nivcsw, p->min_flt, p->maj_flt);
}

static int create_our_cache(void)
{
	int err = 0;

	pr_info("%s: sizeof our ctx structure is %lu bytes\n",
		OURMODNAME, sizeof(struct myctx));
	/* Create a new slab cache:
	 * kmem_cache_create(const char *name, unsigned int size, unsigned int align,
                 slab_flags_t flags, void (*ctor)(void *));
	 */
	gctx_cachep = kmem_cache_create(OURCACHENAME, sizeof(struct myctx),
			sizeof(long),
			SLAB_POISON | SLAB_RED_ZONE | SLAB_HWCACHE_ALIGN,
			our_ctor);
	if (!gctx_cachep) {
		pr_warn("%s:%s():kmem_cache_create() failed\n",
			OURMODNAME, __func__);
		if (IS_ERR(gctx_cachep))
			err = PTR_ERR(gctx_cachep);
		return err;
	}
	
	return 0;
}

static int __init slab_custom_init(void)
{
	pr_debug("%s: inserted\n", OURMODNAME);
	create_our_cache();
	use_our_cache();
	return 0;		/* success */
}

static void __exit slab_custom_exit(void)
{
	kmem_cache_destroy(gctx_cachep);
	pr_debug("%s: removed\n", OURMODNAME);
}

module_init(slab_custom_init);
module_exit(slab_custom_exit);
