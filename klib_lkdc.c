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
#include <asm/io.h>

/* 
 * show_phy - show the virtual, physical addresses and PFNs of the memory
 *            range provided
 * @kaddr: the starting kernel virtual address
 * @len: length of the memory piece (bytes)
 * @contiguity_check: if True, check for physical contiguity of pages
 *
 * 'Walk' the virtually contiguous 'array' of pages one by one (i.e. page by
 * page), printing the virt and physical address (& PFN- page frame number).
 * This way, we can see if the memory really is *physically* contiguous or not
 */
//static void show_phy(void *kaddr, size_t len, bool contiguity_check)
void show_phy(void *kaddr, size_t len, bool contiguity_check)
{
#if(BITS_PER_LONG == 32)
	const char *hdr = "  pg#   va    pa     PFN   (0xPFN)\n";
#else // 64-bit
	const char *hdr = "-pg#-  --------va--------   ----pa----   --PFN- -(0xPFN)-\n";
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
				pr_notice(" *** physical NON-contiguity detected ***\n");
		}

		pr_info("%05d  0x%pK   0x%llx   %ld (0x%lx)\n",
			i, kaddr+(i*PAGE_SIZE), pa, pfn, pfn);
		if (!!contiguity_check)
			prev_pfn = pfn;
	}
}

