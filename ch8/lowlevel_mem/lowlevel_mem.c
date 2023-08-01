/*
 * ch8/lowlevel_mem/lowlevel_mem.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming
 *
 * From: Ch 8: Kernel Memory Allocation for Module Authors, Part 1
 ****************************************************************
 * Brief Description:
 * A quick demo of the essential 'low-level' / page allocator / Buddy System
 * Allocator (BSA) APIs for allocating and freeing memory chunks in kernel
 * space.
 *
 * As an interesting point, we show the virtual, physical addresses and PFN
 * (page frame numbers) of each page in the memory range. The function
 *  show_phy_pages() is in our 'library' code here: ../../klib_llkd.c
 * This way, we can see if the page allocated really are physically
 * contiguous.
 *
 * Also, in the printks below, we use the %[ll]x format specifier in addition
 * to the 'correct' %pK style (for security). We do this here to see the actual
 * virtual addresses (and not some hashed value). Don't do this in production.
 *
 * For details, please refer the book, Ch 8.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include "../../klib_llkd.h"

#define OURMODNAME    "lowlevel_mem"

MODULE_DESCRIPTION("ch8: demo kernel module to exercise essential page allocator APIs");
MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_LICENSE("Dual MIT/GPL");

static const void *gptr1, *gptr2, *gptr3, *gptr4, *gptr5;
static int bsa_alloc_order = 3;
module_param_named(order, bsa_alloc_order, int, 0660);
MODULE_PARM_DESC(order, "order of the allocation (power-to-raise-2-to)");

/*
 * bsa_alloc : test some of the bsa (buddy system allocator
 * aka page allocator) APIs
 */
static int bsa_alloc(void)
{
	int stat = -ENOMEM;
	u64 numpg2alloc = 0;
	const struct page *pg_ptr1;

	/* 0. Show the identity mapping: physical RAM page frames to kernel virtual
	 *    addresses, from PAGE_OFFSET for 5 pages
	 */
	pr_info("%s: 0. Show identity mapping: RAM page frames : kernel virtual pages :: 1:1\n",
		OURMODNAME);
	/* SEE THIS!
	 * Show the virt, phy addr and PFN (page frame numbers).
	 * This function is in our 'library' code here: ../../klib_llkd.c
	 * This way, we can see if the pages allocated are really physically
	 * contiguous. Signature:
	 *  void show_phy_pages(const void *kaddr, size_t len, bool contiguity_check);
	 */
	show_phy_pages((void *)PAGE_OFFSET, 5 * PAGE_SIZE, 1);

	/* 1. Allocate one page with the __get_free_page() API */
	gptr1 = (void *)__get_free_page(GFP_KERNEL);
	if (!gptr1) {
		pr_warn("%s: __get_free_page() failed!\n", OURMODNAME);
		/* As per convention, we emit a printk above saying that the
		 * allocation failed. In practice it isn't required; the kernel
		 * will definitely emit many warning printk's if a memory alloc
		 * request ever fails! Thus, we do this only once (here; could also
         * use the WARN_ONCE()); from now on we don't pedantically print any
		 * error message on a memory allocation request failing.
		 */
		goto out1;
	}
	pr_info("%s: 1. __get_free_page() alloc'ed 1 page from the BSA @ %pK (%px)\n",
		OURMODNAME, gptr1, gptr1);

	/* 2. Allocate 2^bsa_alloc_order pages with the __get_free_pages() API */
	numpg2alloc = powerof(2, bsa_alloc_order);	// returns 2^bsa_alloc_order
	gptr2 = (void *)__get_free_pages(GFP_KERNEL | __GFP_ZERO, bsa_alloc_order);
	if (!gptr2) {
		/* no error/warning printk now; see above comment */
		goto out2;
	}
	pr_info("%s: 2. __get_free_pages() alloc'ed 2^%d = %lld page(s) = %lld bytes\n"
		" from the BSA @ %pK (%px)\n",
		OURMODNAME, bsa_alloc_order, powerof(2, bsa_alloc_order),
		numpg2alloc * PAGE_SIZE, gptr2, gptr2);
	pr_info(" (PAGE_SIZE = %ld bytes)\n", PAGE_SIZE);
	show_phy_pages(gptr2, numpg2alloc * PAGE_SIZE, 1);

	/* 3. Allocate and init one page with the get_zeroed_page() API */
	gptr3 = (void *)get_zeroed_page(GFP_KERNEL);
	if (!gptr3)
		goto out3;
	pr_info("%s: 3. get_zeroed_page() alloc'ed 1 page from the BSA @ %pK (%px)\n",
		OURMODNAME, gptr3, gptr3);

	/* 4. Allocate one page with the alloc_page() API.
	 * Careful! It does not return the alloc'ed page ptr but rather the pointer
	 * to the metadata structure 'page' representing the allocated page:
	 *    struct page * alloc_page(gfp_mask);
	 * So, we use the page_address() helper to convert it to a kernel
	 * logical (or virtual) address.
	 */
	pg_ptr1 = alloc_page(GFP_KERNEL);
	if (!pg_ptr1)
		goto out4;
	gptr4 = page_address(pg_ptr1);
	pr_info("%s: 4. alloc_page() alloc'ed 1 page from the BSA @ %pK (%px)\n"
		" (struct page addr=%pK (%px))\n",
		OURMODNAME, (void *)gptr4, (void *)gptr4, pg_ptr1, pg_ptr1);

	/* 5. Allocate and init 2^3 = 8 pages with the alloc_pages() API.
	 * < Same warning as above applies here too! >
	 */
	gptr5 = page_address(alloc_pages(GFP_KERNEL, 3));
	if (!gptr5)
		goto out5;
	pr_info("%s: 5. alloc_pages() alloc'ed %lld pages from the BSA @ %pK (%px)\n",
		OURMODNAME, powerof(2, 3), (void *)gptr5, (void *)gptr5);

	return 0;
 out5:
	free_page((unsigned long)gptr4);
 out4:
	free_page((unsigned long)gptr3);
 out3:
	free_pages((unsigned long)gptr2, bsa_alloc_order);
 out2:
	free_page((unsigned long)gptr1);
 out1:
	return stat;
}

static int __init lowlevel_mem_init(void)
{
	return bsa_alloc();
}

static void __exit lowlevel_mem_exit(void)
{
	pr_info("%s: free-ing up the BSA memory chunks...\n", OURMODNAME);
	/* Free 'em! We follow the convention of freeing them in the reverse
	 * order from which they were allocated
	 */
	free_pages((unsigned long)gptr5, 3);
	free_page((unsigned long)gptr4);
	free_page((unsigned long)gptr3);
	free_pages((unsigned long)gptr2, bsa_alloc_order);
	free_page((unsigned long)gptr1);
	pr_info("%s: removed\n", OURMODNAME);
}

module_init(lowlevel_mem_init);
module_exit(lowlevel_mem_exit);
