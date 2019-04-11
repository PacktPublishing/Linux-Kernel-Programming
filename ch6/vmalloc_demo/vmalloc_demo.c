/*
 * ch6/vmalloc_demo/vmalloc_demo.c
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
 * A simple demo of using the vmalloc() and friends...
 *
 * For details, please refer the book, Ch 6.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#define OURMODNAME   "vmalloc_demo"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKDC book:ch6/vmalloc_demo/: simple vmalloc() and friends demo lkm");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int kvn = 5*1024*1024;  // 5 MB
module_param(kvn, int, 0644);
MODULE_PARM_DESC(kvn,
 "number of bytes to allocate with the kvmalloc(); (defaults to 5 MB)");

#define KVN_MIN_BYTES    8
#define DISP_BYTES      16 

static void *vptr_rndm, *vptr_init, *kv, *kvarr, *vrx;

static int vmalloc_try(void)
{
	/* 1. vmalloc(); mem contents are random */
	if (!(vptr_rndm = vmalloc(10000))) {
		pr_warn("%s: vmalloc failed\n", OURMODNAME);
		goto err_out1;
	}
	pr_info("vmalloc(): vptr_rndm = %pK\n", vptr_rndm);
	print_hex_dump_bytes(" vptr_rndm: ", DUMP_PREFIX_NONE, vptr_rndm, DISP_BYTES);

	/* 2. vzalloc(); mem contents are set to zeroes */
	if (!(vptr_init = vzalloc(10000))) {
		pr_warn("%s: vzalloc failed\n", OURMODNAME);
		goto err_out2;
	}
	pr_info("vzalloc(): vptr_init = %p (0x%llx)\n", vptr_init, vptr_init);
	print_hex_dump_bytes(" vptr_init: ", DUMP_PREFIX_NONE, vptr_init, DISP_BYTES);

	/* 3. kvmalloc(): allocate 'kvn' bytes with the kvmalloc(); if kvn is
	 * large (enough), this should become a vmalloc() under the hood, else
	 * becomes a kmalloc() */
	if (!(kv = kvmalloc(kvn, GFP_KERNEL))) {
		pr_warn("%s: kvmalloc failed\n", OURMODNAME);
		goto err_out3;
	}
	pr_info("kvmalloc() (%d bytes): kv = %pK\n", kvn, kv);
	print_hex_dump_bytes(" kv: ", DUMP_PREFIX_NONE, kv, KVN_MIN_BYTES);

	/* 4. kcalloc(): allocate an array of 1000 64-bit quantities and zero
	 * out the memory */
	if (!(kvarr = kcalloc(1000, sizeof(u64), GFP_KERNEL))) {
		pr_warn("%s: kvmalloc_array failed\n", OURMODNAME);
		goto err_out4;
	}
	pr_info("kcalloc(): kvarr = %pK\n", kvarr);
	print_hex_dump_bytes(" kvarr: ", DUMP_PREFIX_NONE, kvarr, DISP_BYTES);

	/* 5. __vmalloc(): allocate some 42 pages and set protections to RO */
#undef WR2ROMEM_BUG
/* #define WR2ROMEM_BUG */   /* 'Normal' usage: keep this commented out, else
		      we will crash! Read the book, Ch 6, for details  :-) */
	if (!(vrx = __vmalloc(42*PAGE_SIZE, GFP_KERNEL, PAGE_KERNEL_RO))) {
		pr_warn("%s: __vmalloc failed\n", OURMODNAME);
		goto err_out5;
	}
	pr_info("__vmalloc(): vrx = %pK\n", vrx);

	/* Try reading the memory, should be fine */
	print_hex_dump_bytes(" vrx: ", DUMP_PREFIX_NONE, vrx, DISP_BYTES);
#ifdef WR2ROMEM_BUG
	/* Try writing to the RO memory! We find that the kernel crashes
	 * (emits an Oops!) */
	*(u64 *)(vrx+4) = 0xba;
#endif
	return 0;
err_out5:
	vfree(kvarr);
err_out4:
	vfree(kv);
err_out3:
	vfree(vptr_init);
err_out2:
	vfree(vptr_rndm);
err_out1:
	return -ENOMEM;
}

static int __init vmalloc_demo_init(void)
{
	if (kvn < KVN_MIN_BYTES) {
		pr_info("%s: kvn must be >= %d bytes (curr is %d bytes)\n",
			OURMODNAME, KVN_MIN_BYTES, kvn);
		return -EINVAL;
	}
	pr_debug("%s: inserted\n", OURMODNAME);
	return vmalloc_try();
}

static void __exit vmalloc_demo_exit(void)
{
	vfree(vrx);
	kvfree(kvarr);
	kvfree(kv);
	vfree(vptr_init);
	vfree(vptr_rndm);
	pr_debug("%s: removed\n", OURMODNAME);
}

module_init(vmalloc_demo_init);
module_exit(vmalloc_demo_exit);
