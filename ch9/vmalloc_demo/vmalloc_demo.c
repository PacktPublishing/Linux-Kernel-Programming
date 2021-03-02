/*
 * ch9/vmalloc_demo/vmalloc_demo.c
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
 * A simple demo of using the vmalloc() and friends...
 *
 * For details, please refer the book, Ch 9.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#define OURMODNAME   "vmalloc_demo"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKP book:ch9/vmalloc_demo/: simple vmalloc() and friends demo lkm");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int kvnum = 5 * 1024 * 1024;	// 5 MB
module_param(kvnum, int, 0644);
MODULE_PARM_DESC(kvnum, "number of bytes to allocate with the kvmalloc(); (defaults to 5 MB)");

#define KVN_MIN_BYTES   16
#define DISP_BYTES      16

static void *vptr_rndm, *vptr_init, *kv, *kvarr, *vrx;

static int vmalloc_try(void)
{
	/* 1. vmalloc(); mem contents are random */
	vptr_rndm = vmalloc(10000);
	if (!vptr_rndm) {
		/* The pr_warn() below isn't really required; being pedantic here,
		 * we keep it.. ditto for the remaining cases below...
		 */
		pr_warn("vmalloc failed\n");
		goto err_out1;
	}
	pr_info("1. vmalloc():   vptr_rndm = 0x%pK (actual=0x%px)\n",
		vptr_rndm, vptr_rndm);
	print_hex_dump_bytes(" content: ", DUMP_PREFIX_NONE, vptr_rndm, DISP_BYTES);

	/* 2. vzalloc(); mem contents are set to zeroes */
	vptr_init = vzalloc(10000);
	if (!vptr_init) {
		pr_warn("vzalloc failed\n");
		goto err_out2;
	}
	pr_info("2. vzalloc():   vptr_init = 0x%pK (actual=0x%px)\n",
		vptr_init, vptr_init);
	print_hex_dump_bytes(" content: ", DUMP_PREFIX_NONE, vptr_init, DISP_BYTES);

	/* 3. kvmalloc(): allocate 'kvnum' bytes with the kvmalloc(); if kvnum is
	 * large (enough), this will become a vmalloc() under the hood, else
	 * it fals back to a kmalloc()
	 */
	kv = kvmalloc(kvnum, GFP_KERNEL);
	if (!kv) {
		pr_warn("kvmalloc failed\n");
		goto err_out3;
	}
	pr_info("3. kvmalloc() :        kv = 0x%pK (actual=0x%px)\n"
			"    (for %d bytes)\n", kv, kv, kvnum);
	print_hex_dump_bytes(" content: ", DUMP_PREFIX_NONE, kv, KVN_MIN_BYTES);

	/* 4. kcalloc(): allocate an array of 1000 64-bit quantities and zero
	 * out the memory */
	kvarr = kcalloc(1000, sizeof(u64), GFP_KERNEL);
	if (!kvarr) {
		pr_warn("kcalloc failed\n");
		goto err_out4;
	}
	pr_info("4. kcalloc() :      kvarr = 0x%pK (actual=0x%px)\n", kvarr, kvarr);
	print_hex_dump_bytes(" content: ", DUMP_PREFIX_NONE, kvarr, DISP_BYTES);

	/* 5. __vmalloc(): allocate some 42 pages and set protections to RO */
#undef WR2ROMEM_BUG
	/* #define WR2ROMEM_BUG */
	/* 'Normal' usage: keep this commented out, else
	 * we will crash! Read the book, Ch 9, for details  :-) */
	vrx = __vmalloc(42 * PAGE_SIZE, GFP_KERNEL, PAGE_KERNEL_RO);
	if (!vrx) {
		pr_warn("__vmalloc failed\n");
		goto err_out5;
	}
	pr_info("5. __vmalloc():       vrx = 0x%pK (actual=0x%px)\n", vrx, vrx);

	/* Try reading the memory, should be fine */
	print_hex_dump_bytes(" content: ", DUMP_PREFIX_NONE, vrx, DISP_BYTES);
#ifdef WR2ROMEM_BUG
	/* Try writing to the RO memory! We find that the kernel crashes
	 * (emits an Oops!) */
	*(u64 *) (vrx + 4) = 0xba;
#endif
	return 0;		/* success */
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
	if (kvnum < KVN_MIN_BYTES) {
		pr_info("kvnum must be >= %d bytes (curr it's %d bytes)\n", KVN_MIN_BYTES, kvnum);
		return -EINVAL;
	}
	pr_info("inserted\n");

	return vmalloc_try();
}

static void __exit vmalloc_demo_exit(void)
{
	vfree(vrx);
	kvfree(kvarr);
	kvfree(kv);
	vfree(vptr_init);
	vfree(vptr_rndm);
	pr_info("removed\n");
}

module_init(vmalloc_demo_init);
module_exit(vmalloc_demo_exit);
