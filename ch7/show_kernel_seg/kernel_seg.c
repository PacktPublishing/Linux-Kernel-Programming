/*
 * ch7/kernel_seg/kernel_seg.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 7: Kernel and Memory Management Internals Essentials
 ****************************************************************
 * Brief Description:
 * A kernel module to show us some relevant details wrt the layout of the
 * kernel segment, IOW, the kernel VAS (Virtual Address Space). In effect,
 * this shows a simple memory map of the kernel. Works on both 32 and 64-bit
 * systems of differing architectures (note: only lightly tested on ARM and
 * x86 32 and 64-bit systems).
 * Optionally also displays key info of the user VAS if the module parameter
 * show_uservas is set to 1.
 *
 * Useful! With show_uservas=1 we literally 'see' the full memory map of the
 * process, including kernel-space.
 * (Also, fyi, for a more detailed view of the kernel/user VAS, check out the
 * 'procmap' utility).
 *
 * For details, please refer the book, Ch 7.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#include <asm/pgtable.h>
#include "../../klib_llkd.h"
#include "../../convenient.h"

#define OURMODNAME   "kernel_seg"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LLKD book:ch7/kernel_seg: display some kernel segment details");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/* Module parameters */
static int show_uservas;
module_param(show_uservas, int, 0660);
MODULE_PARM_DESC(show_uservas, "Show some user space VAS details; 0 = no (default), 1 = show");

#if (BITS_PER_LONG == 32)
#define FMTSPC_DEC	"%7d"
#elif(BITS_PER_LONG == 64)
#define FMTSPC_DEC	"%9ld"
#endif

#define ELLPS "|                           [ . . . ]                         |\n"

extern void llkd_minsysinfo(void);	// it's in our klib_llkd 'library'

/*
 * show_userspace_info
 * Display some arch-independent details of the usermode VAS.
 * Format (for most of the details):
 *  |<name of region>:   start_addr - end_addr        | [ size in KB/MB/GB]
 *
 * f.e. on an x86_64 VM w/ 2047 MB RAM
 *  | text segment  0x0000563022f3ed90 - 0x0000563022f1c000 | [   142736 bytes]
 * We order it by descending address (here, uva's).
 */
static void show_userspace_info(void)
{
	pr_info("+------------ Above is kernel-seg; below, user VAS  ----------+\n"
		ELLPS
		"|Process environment "
		" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " bytes]\n"
		"|          arguments "
		" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " bytes]\n"
		"|        stack start  0x" FMTSPC "                      |\n"
		"|       heap segment "
		" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " bytes]\n"
		"|static data segment "
		" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " bytes]\n"
		"|       text segment "
		" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " bytes]\n"
		ELLPS
		"+-------------------------------------------------------------+\n",
		SHOW_DELTA_b((TYPECST) current->mm->env_start, (TYPECST) current->mm->env_end),
		SHOW_DELTA_b((TYPECST) current->mm->arg_start, (TYPECST) current->mm->arg_end),
		(TYPECST) current->mm->start_stack,
		SHOW_DELTA_b((TYPECST) current->mm->start_brk, (TYPECST) current->mm->brk),
		SHOW_DELTA_b((TYPECST) current->mm->start_data,
			     (TYPECST) current->mm->end_data),
		SHOW_DELTA_b((TYPECST) current->mm->start_code,
			     (TYPECST) current->mm->end_code)
	    );

	pr_info(
#if (BITS_PER_LONG == 64)
		       "Above: TASK_SIZE         = 0x" FMTSPC " size of userland  [  "
		       FMTSPC_DEC " GB]\n"
#else				// 32-bit
		       "Above: TASK_SIZE         = 0x" FMTSPC " size of userland  [  "
		       FMTSPC_DEC " MB]\n"
#endif
		       " # userspace memory regions (VMAs) = %d\n"
		       " Above statistics are wrt 'current' thread (see below):\n",
#if (BITS_PER_LONG == 64)
		       (TYPECST) TASK_SIZE, (TYPECST) (TASK_SIZE >> 30),
#else				// 32-bit
		       (TYPECST) TASK_SIZE, (TYPECST) (TASK_SIZE >> 20),
#endif
		       current->mm->map_count);

	PRINT_CTX();		/* show which process is the one in context */
}

/*
 * show_kernelseg_info
 * Display kernel segment details as applicable to the architecture we're
 * currently running upon.
 * Format (for most of the details):
 *  |<name of region>:   start_addr - end_addr        | [ size in KB/MB/GB]
 *
 * f.e. on an x86_64 VM w/ 2047 MB RAM
 *  |lowmem region:   0xffffa0dfc0000000 - 0xffffa0e03fff0000 | [ 2047 MB = 1 GB]
 * We try to order it by descending address (here, kva's) but this doesn't
 * always work out as ordering of regions differs by arch.
 */
static void show_kernelseg_info(void)
{
	pr_info("\nSome Kernel Details [by decreasing address]\n"
		"+-------------------------------------------------------------+\n");
#ifdef CONFIG_ARM
	/* On ARM, the definition of VECTORS_BASE turns up only in kernels >= 4.11 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 11, 0)
	pr_info("|vector table:       "
		" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " KB]  |\n",
		SHOW_DELTA_K((TYPECST) VECTORS_BASE, (TYPECST) VECTORS_BASE + PAGE_SIZE));
#endif
#endif

	/* kernel fixmap region */
	pr_info(ELLPS
		"|fixmap region:      "
		" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " MB]  |\n",
#ifdef CONFIG_ARM
		/* RELOOK: We seem to have an issue on ARM; the compile fails with:
		 *  "./include/asm-generic/fixmap.h:29:38: error: invalid storage
		 *   class for function ‘fix_to_virt’"
		 * ### So, okay, as a *really silly* workaround am simply copying in the
		 * required macros from the arch/arm/include/asm/fixmap.h header
		 * manually here ###
		 */
#define FIXADDR_START   0xffc00000UL
#define FIXADDR_END     0xfff00000UL
		SHOW_DELTA_M((TYPECST) FIXADDR_START, (TYPECST) FIXADDR_END));
#else
#include <asm/fixmap.h>
		// seems to work fine on x86
		SHOW_DELTA_M((TYPECST) FIXADDR_START, (TYPECST) FIXADDR_START + FIXADDR_SIZE));
#endif

	/* kernel module region
	 * For the modules region, it's high in the kernel segment on typical 64-bit
	 * systems, but the other way around on many 32-bit systems (particularly
	 * ARM-32); so we rearrange the order in which it's shown depending on the
	 * arch, thus trying to maintain a 'by descending address' ordering.
	 */
#if (BITS_PER_LONG == 64)
	pr_info("|module region:      "
		" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " MB]  |\n",
		SHOW_DELTA_M((TYPECST) MODULES_VADDR, (TYPECST) MODULES_END));
#endif

#ifdef CONFIG_KASAN		// KASAN region: Kernel Address SANitizer
	pr_info("|KASAN shadow:       "
		" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " GB]\n",
		SHOW_DELTA_G((TYPECST) KASAN_SHADOW_START, (TYPECST) KASAN_SHADOW_END));
#endif

	/* vmalloc region */
	pr_info("|vmalloc region:     "
		" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " MB = " FMTSPC_DEC " GB]\n",
		SHOW_DELTA_MG((TYPECST) VMALLOC_START, (TYPECST) VMALLOC_END));

	/* lowmem region */
	pr_info("|lowmem region:      "
		" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " MB = " FMTSPC_DEC " GB]"
#if (BITS_PER_LONG == 32)
		"\n|             (above:PAGE_OFFSET - highmem)                   |\n",
#else
		"\n|                  (above:PAGE_OFFSET    -      highmem)      |\n",
#endif
		SHOW_DELTA_MG((TYPECST) PAGE_OFFSET, (TYPECST) high_memory));

	/* (possible) highmem region;  may be present on some 32-bit systems */
#ifdef CONFIG_HIGHMEM
	pr_info("|HIGHMEM region:     "
		" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " MB]\n",
		SHOW_DELTA_M((TYPECST) PKMAP_BASE,
			     (TYPECST) (PKMAP_BASE) + (LAST_PKMAP * PAGE_SIZE)));
#endif

	/*
	 * Symbols for kernel:
	 *   text begin/end (_text/_etext)
	 *   init begin/end (__init_begin, __init_end)
	 *   data begin/end (_sdata, _edata)
	 *   bss begin/end (__bss_start, __bss_stop)
	 * are only defined *within* (in-tree) and aren't available for modules;
	 * thus we don't attempt to print them.
	 */

#if (BITS_PER_LONG == 32)	/* modules region: see the comment above reg this */
	pr_info("|module region:      "
		" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " MB]  |\n",
		SHOW_DELTA_M((TYPECST) MODULES_VADDR, (TYPECST) MODULES_END));
#endif
	pr_info(ELLPS);
}

static int __init kernel_seg_init(void)
{
	pr_info("%s: inserted\n", OURMODNAME);

	/* Display some minimal system info
	 * Note: this function is within our kernel 'library' here:
	 *  ../../llkd_klib.c
	 * Hence, we must arrange to link it in (see the Makefile)
	 */
	llkd_minsysinfo();
	show_kernelseg_info();

	if (show_uservas)
		show_userspace_info();
	else {
		pr_info("+-------------------------------------------------------------+\n");
		pr_info("%s: skipping show userspace...\n", OURMODNAME);
	}

	return 0;		/* success */
}

static void __exit kernel_seg_exit(void)
{
	pr_info("%s: removed\n", OURMODNAME);
}

module_init(kernel_seg_init);
module_exit(kernel_seg_exit);
