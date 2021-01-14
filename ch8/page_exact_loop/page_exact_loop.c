/*
 * ch8/page_exact_loop/page_exact_loop.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 8 : Kernel Memory Allocation for Module Authors, Part 1
 ****************************************************************
 * Brief Description:
 * A quick demo of using the alloc_pages_exact() and it's counterpart
 * the free_pages_exact() APIs; they're more optimized when allocating
 * larger memory chunks via the page allocator.
 * (But pl read the rest of the chapter and Ch 9 as well!).
 *
 * For details, please refer the book, Ch 8.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>

#define OURMODNAME   "page_exact_loop"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION
("LLKD ch8/page_exact_loop: demo using the superior [alloc|free]_pages_exact() APIs");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/*
#if (BITS_PER_LONG == 32)
#define FMTSPC "0x%08x"
#define TYPECST unsigned int
#elif(BITS_PER_LONG == 64)
#define FMTSPC "0x%016lx"
#define TYPECST unsigned long
#endif
*/
#define MAXTIMES    5 	/* the higher you make this, the more the chance of the
						 * alloc failing, as we only free in the cleanup code path...
						 */
void show_phy_pages(const void *kaddr, size_t len, bool contiguity_check);

static void *gptr[MAXTIMES];
/* Lets ask for a little over 32 KB; the 'regular' PA/BSA APIs will end up giving
 * us 64 KB; the alloc_pages_exact() will optimize it
 */
static size_t gsz = 32*1024 + 5120; //9 * 1024;
//static size_t gsz = 4 * 1024 * 1024;
	/* 4 MB; the largest possible alloc w/ a
	 * single call to the page allocator is 4 MB (assuming MAX_ORDER of 11 and
	 * page size of 4096)
	 */

static int __init page_exact_loop_init(void)
{
	int i, j;

	pr_info("%s: inserted\n", OURMODNAME);

	for (i = 0; i < MAXTIMES; i++) {
		gptr[i] = alloc_pages_exact(gsz, GFP_KERNEL);
		if (!gptr[i]) {
			pr_warn("%s: alloc_pages_exact() failed! (loop index %d)\n",
				OURMODNAME, i);
			/* it failed; don't leak, ensure we free the memory taken so far! */
			for (j = i; j >= 0; j--)
				free_pages_exact(gptr[j], gsz);
			return -ENOMEM;
		}
		pr_info("%s:%d: alloc_pages_exact() alloc'ed %zu bytes memory (%zu pages)"
			" from the BSA @ %pK (actual=%px)\n",
			OURMODNAME, i, gsz, gsz / PAGE_SIZE, gptr[i], gptr[i]);
		// lets 'poison' it..
		memset(gptr[i], 'x', gsz);

		show_phy_pages(gptr[i], gsz, 1);
		msleep(100);
	}

	return 0;		/* success */
}

static void __exit page_exact_loop_exit(void)
{
	int i;

	for (i = 0; i < MAXTIMES; i++)
		free_pages_exact(gptr[i], gsz);
	pr_info("%s: mem freed, removed\n", OURMODNAME);
}

module_init(page_exact_loop_init);
module_exit(page_exact_loop_exit);
