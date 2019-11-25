/*
 * ch7/page_exact/page_exact.c
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

#define OURMODNAME   "page_exact"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LLKD book:ch5/: demo using the superior [alloc|free]_pages_exact() BSA APIs");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

void show_phy_pages(const void *kaddr, size_t len, bool contiguity_check);

static const size_t gsz = 161*PAGE_SIZE;
static void *gptr;

static int __init page_exact_init(void)
{
	pr_debug("%s: inserted\n", OURMODNAME);
	gptr = alloc_pages_exact(gsz, GFP_KERNEL);
	if (!gptr) {
		pr_warn("%s: alloc_pages_exact() failed!\n", OURMODNAME);
		return -ENOMEM;
	}
	pr_info("%s: alloc_pages_exact() alloc'ed %zu bytes memory from the BSA @ %pK\n",
		OURMODNAME, gsz, gptr);
	// lets 'poison' it..
	memset(gptr, 'x', gsz);

	show_phy_pages(gptr, gsz, 1);

	return 0;		/* success */
}

static void __exit page_exact_exit(void)
{
	free_pages_exact(gptr, gsz);
	pr_debug("%s: mem freed, removed\n", OURMODNAME);
}

module_init(page_exact_init);
module_exit(page_exact_exit);
