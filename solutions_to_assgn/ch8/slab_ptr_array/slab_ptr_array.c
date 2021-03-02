/*
 * solutions_to_assgn/ch8/slab_ptr_array/slab_ptr_array.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming
 *
 * From: Ch 8 : Kernel Memory Allocation for Module Authors Part 1
 ****************************************************************
 * Brief Description:
 * Assignment:
 * Write a kernel module, that, within it's init code, allocates slab memory
 * with the kmalloc() to an array of five pointers, i.e., each pointer gets a
 * kilobyte of slab memory,like this:
 *   +-------+-------+-------+-------+-------+
 *   |0  .   |1  .   |2  .   |3  .   |4  .   |
 *   +---|---+---|---+---|---+---|---+---|---+
 *       v       v       v       v       v
 *      ---     ---     ---     ---     ---
 *      mem     mem     mem     mem     mem       <-- 1 KB each
 *      ---     ---     ---     ---     ---
 * Then, initialize the buffers to 'a's, 'b's, 'c's, 'd's and 'e's
 * respectively. In the cleanup code, free up the memory.
 * Tip: take care to handle the error case where a memory allocation fails;
 * don't forget to free the allocations already performed!
 *
 * For details, please refer the book, Ch 8.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#define OURMODNAME       "slab_ptr_array"
#define SLAB_MAXLOOP    5

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKDC book:solutions_to_assgn/ch8/slab_ptr_array/: assignment solution");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static char *gkptr[SLAB_MAXLOOP];

static int __init slab_ptr_array_init(void)
{
	int i = 0, j;

	pr_info("%s: inserted\n", OURMODNAME);

	while (i < SLAB_MAXLOOP) {
		gkptr[i] = kmalloc(1024, GFP_KERNEL);
		if (!gkptr[i]) {
			// pedantic warning, unnecessary in production code
			pr_warn("%s: kmalloc iter %d failed!\n", OURMODNAME, i);
			/* Careful! take care to free the prev allocated mem, if any,
			 * thus avoiding possible memory leakage !
			 */
			goto cleanup;
		}
		memset(gkptr[i], i+0x61, 1024); // 0x61 = 'a'
		print_hex_dump_bytes("gkptr: ", DUMP_PREFIX_NONE, gkptr[i], 16);
		i++;
	}

	return 0;		/* success */
cleanup:
	for (j = i; j > 0; j--) {
		pr_debug(" freeing gkptr[%d]\n", j);
		kfree(gkptr[j]);
	}
	return -ENOMEM;
}

static void __exit slab_ptr_array_cleanup(void)
{
	int i;

	for (i = 0; i < SLAB_MAXLOOP; i++) {
		pr_debug("%s:%s(): freeing gkptr[%d]\n",
			OURMODNAME, __func__, i);
		kfree(gkptr[i]);
	}
	pr_info("%s: freed memory, removed\n", OURMODNAME);
}

module_init(slab_ptr_array_init);
module_exit(slab_ptr_array_cleanup);
