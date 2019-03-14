/*
 * ch4/page_exact/page_exact.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch 4 : Kernel Memory Allocation for Module Authors
 ****************************************************************
 * Brief Description:
 *
 * For details, please refer the book, Ch 4.
 */
#include <linux/init.h>
#include <linux/module.h>
//#include <linux/types.h>
#include <asm/io.h>

#define OURMODNAME   "page_exact"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKDC book:ch4/: demo using the superior [alloc|free]_pages_exact() BSA APIs");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static const size_t gsz = 161*PAGE_SIZE;
static void *gptr;

/* 'Walk' the virtually contiguous 'array' of pages one by one (i.e. page by
 * page), printing the virt and physical address (& PFN- page frame number).
 * This way, we can see if the memory really is *physically* contiguous or not
 *
 * Parameters:
 * kaddr    : the starting kernel virtual address
 * len      : length of the memory piece (bytes)
 */
static void show_phy(void *kaddr, size_t len, bool contiguity_check)
{
#if(BITS_PER_LONG == 32)
	const char *hdr = "  pg#   va    pa     PFN   (0xPFN)\n";
#else // 64-bit
	const char *hdr = "-pg#-  --------va--------   ----pa----   -PFN-- -(0xPFN)-\n";
#endif
	phys_addr_t pa;
	int i;
	long pfn, prev_pfn = 1;
	
	if (!virt_addr_valid(kaddr))
		return;

	pr_info("%s", hdr);
	for (i = 0; i < len/PAGE_SIZE; i++) {
		pa = virt_to_phys(kaddr+(i*PAGE_SIZE));
		pfn = PHYS_PFN(pa);

		if (!!contiguity_check) {
			if (i && pfn != prev_pfn + 1)
				pr_info(" *** physical NON-contiguity detected ***\n");
		}

		pr_info("%05d  0x%pK   0x%llx   %ld (0x%lx)\n",
			i, kaddr+(i*PAGE_SIZE), pa, pfn, pfn);
		if (!!contiguity_check)
			prev_pfn = pfn;
	}
}

static int __init page_exact_init(void)
{
	pr_debug("%s: inserted\n", OURMODNAME);
	gptr = alloc_pages_exact(gsz, GFP_KERNEL);
	if (!gptr) {
		pr_warn("%s: alloc_pages_exact() failed!\n", OURMODNAME);
		return -ENOMEM;
	}
	pr_info("%s: alloc_pages_exact() alloc'ed %ld bytes memory from the BSA @ %pK\n",
		OURMODNAME, gsz, gptr);
	// lets 'poison' it..
	memset(gptr, 'x', gsz);

	show_phy(gptr, gsz, 1);

	return 0;		/* success */
}

static void __exit page_exact_exit(void)
{
	free_pages_exact(gptr, gsz);
	pr_debug("%s: mem freed, removed\n", OURMODNAME);
}

module_init(page_exact_init);
module_exit(page_exact_exit);
