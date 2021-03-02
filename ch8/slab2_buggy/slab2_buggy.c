/*
 * ch8/slab2_buggy/slab2_buggy.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming
 *
 * From: Ch 8 : Kernel Memory Allocation for Module Authors, Part 1
 ****************************************************************
 * Brief Description:
 * If you set the macro BUGGY to 1 (it's 0 by default), then a dangerous bug
 * surfaces! Check it out - in the init code, on the second loop iteration,
 * the kmalloc() will not be invoked as the if (kptr == NULL) condition will
 * evaluate to false; but, the kfree() is still invoked, now on an invalid
 * parameter (it actually becomes a double-free bug)! On our test system,
 * the entire machine just froze.
 *
 * For details, please refer the book, Ch 8.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

#define OURMODNAME   "slab2_buggy"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKP book:ch8/slab2_buggy: kmalloc/kfree bug demo");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int __init slab2_buggy_init(void)
{
	int i = 0;
	char *kptr = NULL;

	pr_info("%s: inserted\n", OURMODNAME);

#define BUGGY    0		/* by default it's set to 0 to rm the bug(s) :-) */

	while (++i < 4) {
#if (BUGGY == 1)
		if (kptr == NULL)
#endif
			kptr = kmalloc(1024, GFP_KERNEL);
		if (!kptr) {
			pr_warn("%s: kmalloc failed!\n", OURMODNAME);
			/* Bug: we don't free the prev allocated mem, if any,
			 * resulting in possible memory leakage !
			 */
			return -ENOMEM;
		}
		memset(kptr, i - 1 + 0x61, 1024);	// 0x61 = 'a'
		print_hex_dump_bytes("kptr: ", DUMP_PREFIX_OFFSET, kptr, 16);
		kfree(kptr);
#if (BUGGY == 1)
		pr_info(" %d: kfree on %pK\n", i, kptr);
#endif
	}

	return 0;		/* success */
}

static void __exit slab2_buggy_exit(void)
{
	pr_info("%s: removed\n", OURMODNAME);
}

module_init(slab2_buggy_init);
module_exit(slab2_buggy_exit);
