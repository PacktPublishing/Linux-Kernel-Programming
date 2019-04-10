/*
 * ch4/slab5_actualsize/slab5_actualsize.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch 4 : Linux Kernel Memory Allocation for Module Authors 
 ****************************************************************
 * Brief Description:
 *
 * For details, please refer the book, Ch 4.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

#define OURMODNAME   "slab5_actualsize"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKDC book:ch4/slab5_actualsize: test slab alloc with the ksize()");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int stepsz = 200000;
module_param(stepsz, int, 0644);
MODULE_PARM_DESC(stepsz,
 "Amount to increase allocation by on each loop iteration (default=200000");

static int test_maxallocsz(void)
{
	/* This time, initialize size2alloc to 100, as otherwise we'll get a
	 * divide error! */
	size_t size2alloc = 100, actual_alloc;
	void *p;

	while (1) {
		p = kmalloc(size2alloc, GFP_KERNEL);
		if (!p) {
			pr_alert("kmalloc fail, size2alloc=%ld\n", size2alloc);
			return -ENOMEM;
		}
		actual_alloc = ksize(p);
		/* Print the size2alloc, the amount actually allocated,
	         * the delta between the two, and the percentage of waste
		 */
		pr_info("kmalloc(%7ld) : %7ld : %7ld : %3ld%%\n",
                        size2alloc, actual_alloc, (actual_alloc-size2alloc),
			(((actual_alloc-size2alloc)*100/size2alloc)));
		kfree(p);
		size2alloc += stepsz;
	}
	return 0;
}

static int __init slab5_actualsize_init(void)
{
	pr_debug("%s: inserted\n", OURMODNAME);
	return test_maxallocsz();
}
static void __exit slab5_actualsize_exit(void)
{
	pr_debug("%s: removed\n", OURMODNAME);
}

module_init(slab5_actualsize_init);
module_exit(slab5_actualsize_exit);
