/*
 * klib_lkdc.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 ****************************************************************
 * Brief Description:
 * This kernel (module) code is meant to serve as a 'library' of sorts. Other
 * kernel modules in our codebase might wish to link into it and use it's code.
 *
 * For details, please refer the book.
 */
#include <linux/init.h>
#include <linux/module.h>

/* 
 * show_phy_pages - show the virtual, physical addresses and PFNs of the memory
 *            range provided on a per-page basis.
 * @kaddr: the starting kernel virtual address
 * @len: length of the memory piece (bytes)
 * @contiguity_check: if True, check for physical contiguity of pages
 *
 * 'Walk' the virtually contiguous 'array' of pages one by one (i.e. page by
 * page), printing the virt and physical address (& PFN- page frame number).
 * This way, we can see if the memory really is *physically* contiguous or not
 */
void show_phy_pages(const void *kaddr, size_t len, bool contiguity_check)
{
	void *vaddr = kaddr;
#if(BITS_PER_LONG == 64)
	const char *hdr = "-pg#-  --------va--------   ----pa----   -PFN--\n";
#else // 32-bit
	const char *hdr = "-pg#-  ----va----   ----pa----   -PFN--\n";
#endif
	phys_addr_t pa;
	int i;
	long pfn, prev_pfn = 1;
	
#ifdef CONFIG_X86
	if (!virt_addr_valid(vaddr))
		return;
#endif

	pr_info("%s", hdr);
	for (i = 0; i < len/PAGE_SIZE; i++) {
		pa = virt_to_phys(vaddr+(i*PAGE_SIZE));
		pfn = PHYS_PFN(pa);

		if (!!contiguity_check) {
			if (i && pfn != prev_pfn + 1)
				pr_notice(" *** physical NON-contiguity detected ***\n");
		}

		/* Below we show the actual virt addr and not a hashed value by
		 * using the 0x%[ll]x format specifier instead of the %pK as we
		 * should for security */
#if(BITS_PER_LONG == 64)
		pr_info("%05d  0x%016llx   0x%llx   %ld\n",
#else   // 32-bit
		pr_info("%05d  0x%08x   0x%x   %ld\n",
#endif
			i, vaddr+(i*PAGE_SIZE), pa, pfn);
		if (!!contiguity_check)
			prev_pfn = pfn;
	}
}

