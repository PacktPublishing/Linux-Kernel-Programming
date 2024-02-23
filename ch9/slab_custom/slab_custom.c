/*
 * ch9/slab_custom/slab_custom.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming
 *
 * From: Ch 9 : Kernel Memory Allocation for Module Authors, Part 2
 ****************************************************************
 * Brief Description:
 * Simple demo of using the slab layer (exorted) APIs to create our very own
 * custom slab cache.
 *
 * For details, please refer the book, Ch 9.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/sched.h>	/* current */

#define OURMODNAME   "slab_custom"
#define OURCACHENAME "our_ctx"

static int use_ctor = 1;
module_param(use_ctor, uint, 0);
MODULE_PARM_DESC(use_ctor, "if set to 1 (default), our custom ctor routine"
" will initialize slabmem; when 0, no custom constructor will run");

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKP book:ch9/slab_custom: simple demo of creating a custom slab cache");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/* Our 'demo' structure; one that (we imagine) is often allocated and freed;
 * hence, we create a custom slab cache to hold pre-allocated 'instances'
 * of it... Size of one structure instance: 328 bytes.
 */
struct myctx {
	u32 iarr[10];
	u64 uarr[10];
	char uname[128], passwd[16], config[64];
};
static struct kmem_cache *gctx_cachep;

static int use_our_cache(void)
{
	struct myctx *obj = NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 39)
	pr_debug("Cache name is %s\n", kmem_cache_name(gctx_cachep));
#else
	pr_debug("[ker ver > 2.6.38 cache name deprecated...]\n");
#endif

	obj = kmem_cache_alloc(gctx_cachep, GFP_KERNEL);
	if (!obj) {		/* pedantic warning printk below... */
		pr_warn("[Pedantic] kmem_cache_alloc() failed\n");
		return -ENOMEM;
	}

	pr_info("Our cache object (@ %pK, actual=%px) size is %u bytes; actual ksize=%zu\n",
		obj, obj, kmem_cache_size(gctx_cachep), ksize(obj));
	print_hex_dump_bytes("obj: ", DUMP_PREFIX_OFFSET, obj, sizeof(struct myctx));

	/* free it */
	kmem_cache_free(gctx_cachep, obj);
	return 0;
}

/* The parameter is the pointer to the just allocated memory 'object' from
 * our custom slab cache; here, this is our 'constructor' routine; so, we
 * initialize our just allocated memory object.
 */
static void our_ctor(void *new)
{
	struct myctx *ctx = new;
	struct task_struct *p = current;

	/* TIP: to see how exactly we got here, insert this call:
	 *  dump_stack();
	 * (read it bottom-up ignoring call frames that begin with '?')
	 */
	pr_info("in ctor: just alloced mem object is @ 0x%px\n", ctx);	/* %pK in production */
	memset(ctx, 0, sizeof(struct myctx));

	/* As a demo, we init the 'config' field of our structure to some
	 * (arbitrary) 'accounting' values from our task_struct
	 */
	snprintf(ctx->config, 6 * sizeof(u64) + 5, "%d.%d,%ld.%ld,%ld,%ld",
		 p->tgid, p->pid, p->nvcsw, p->nivcsw, p->min_flt, p->maj_flt);
}

static int create_our_cache(void)
{
	int ret = 0;
	void *ctor_fn = NULL;

	if (use_ctor == 1)
		ctor_fn = our_ctor;

	pr_info("sizeof our ctx structure is %zu bytes\n"
		" using custom constructor routine? %s\n",
		sizeof(struct myctx), use_ctor == 1 ? "yes" : "no");

	/* Create a new slab cache:
	 * kmem_cache_create(const char *name, unsigned int size, unsigned int align,
	 slab_flags_t flags, void (*ctor)(void *));
	 */
	gctx_cachep = kmem_cache_create(OURCACHENAME,	// name of our cache
					sizeof(struct myctx), // (min) size of each object
					sizeof(long),		  // alignment
					SLAB_POISON |   /* use slab poison values (explained soon) */
					SLAB_RED_ZONE | /* good for catching buffer under|over-flow bugs */
					SLAB_HWCACHE_ALIGN, /* good for performance */
					ctor_fn);	// ctor: here, on by default
	if (!gctx_cachep) {
		/* When a mem alloc fails we'll usually not require a warning
		 * message as the kernel will definitely emit warning printk's
		 * We do so here pedantically...
		 */
		pr_warn("kmem_cache_create() failed\n");
		if (IS_ERR(gctx_cachep))
			ret = PTR_ERR(gctx_cachep);
	}

	return ret;
}

static int __init slab_custom_init(void)
{
	pr_info("inserted\n");
	create_our_cache();
	return use_our_cache();
}

static void __exit slab_custom_exit(void)
{
	kmem_cache_destroy(gctx_cachep);
	pr_info("custom cache destroyed; removed\n");
}

module_init(slab_custom_init);
module_exit(slab_custom_exit);
