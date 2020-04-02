/*
 * ch6/kernel_seg/kernel_seg.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 6: Kernel and Memory Management Internals Essentials
 ****************************************************************
 * Brief Description:
 * A kernel module to show us stuff regarding the layout of the kernel segment;
 * the kernel VAS (Virtual Address Space). In effect, showing us a simple memory
 * map of the kernel. Works on both 32 and 64-bit systems of differing
 * architectures (CPUs; note, though, this is only lightly tested on ARM and
 * x86 32 and 64-bit systems).
 * Optionally also displays key info of the user VAS if the module parameter
 * show_uservas is set to 1.
 *
 * For details, please refer the book, Ch 6.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/pgtable.h>
#include "../../klib_llkd.h"
#include "../../convenient.h"

#define OURMODNAME   "kernel_seg"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION(
	"LLKD book:ch6/kernel_seg: display some kernel segment details");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/* Module parameters */
static int show_uservas;
module_param(show_uservas, int, 0660);
MODULE_PARM_DESC(show_uservas,
        "Show some userspace VAS details; 0 = no (default), 1 = show");

#if(BITS_PER_LONG == 32)
	#define FMTSPC		"%08x"
	#define FMTSPC_DEC	"%7d"
	#define TYPECST		unsigned int
#elif(BITS_PER_LONG == 64)
	#define FMTSPC		"%016lx"
	#define FMTSPC_DEC	"%9ld"
	#define TYPECST	        unsigned long
#endif

#define ELLPS "|                           [ . . . ]                         |\n"

extern void llkd_minsysinfo(void);	// it's in our 'library'

/* 
 * show_userspace_info
 * Display some arch-independent details of the usermode VAS
 */
static void show_userspace_info(void)
{
	pr_info (
	"+------------ Above, kernel-space; Below, userspace ----------+\n"
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
		(TYPECST)current->mm->env_end,
		(TYPECST)current->mm->env_start,
		(TYPECST)(current->mm->env_end-current->mm->env_start),
		(TYPECST)current->mm->arg_end, 
		(TYPECST)current->mm->arg_start,
		(TYPECST)(current->mm->arg_end-current->mm->arg_start),
		(TYPECST)current->mm->start_stack,
		(TYPECST)current->mm->brk,
		(TYPECST)current->mm->start_brk,
		(TYPECST)(current->mm->brk-current->mm->start_brk),
		(TYPECST)current->mm->end_data,
		(TYPECST)current->mm->start_data,
		(TYPECST)(current->mm->end_data-current->mm->start_data),
		(TYPECST)current->mm->end_code,
		(TYPECST)current->mm->start_code,
		(TYPECST)(current->mm->end_code-current->mm->start_code)
		);

	pr_info(
	"Above: TASK_SIZE         = 0x" FMTSPC " size of userland  [  " FMTSPC_DEC " GB]\n"
	" # userspace memory regions (VMAs) = %d\n"
	" Above statistics are wrt 'current' thread (see below):\n",
		(TYPECST)TASK_SIZE, (TYPECST)(TASK_SIZE >> 30),
		current->mm->map_count);

	PRINT_CTX();       /* show which process is the one in context;
			    * see the definition in the convenient.h header */
}

/* 
 * show_kernelseg_info
 * Display kernel segment details as applicable to the architecture we're
 * currently running upon.
 * We try to order it by descending address but this doesn't always work out
 * as ordering of regions differs by arch.
 */
static void show_kernelseg_info(void)
{
	pr_info("\nSome Kernel Details [by decreasing address]\n"
	"+-------------------------------------------------------------+\n");
#ifdef ARM
	pr_info(
	"|vector table:       "
	" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " KB]\n",
		SHOW_DELTA_K((TYPECST)VECTORS_BASE, (TYPECST)VECTORS_BASE+PAGE_SIZE));
#endif

	/* kernel fixmap region */
	pr_info(
	ELLPS
	"|fixmap region:      "
	" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " MB]\n",
#ifdef CONFIG_ARM
	/* We seem to have an issue on ARM; the compile fails with:
	 *  "./include/asm-generic/fixmap.h:29:38: error: invalid storage
	 *   class for function ‘fix_to_virt’"
	 * ### So, okay, as a *really silly* workaround am simply copying in the
	 * required macros from the fixmap.h header manually here ###
	 * (seems to work fine on x86).
	 */
#define FIXADDR_START   0xffc00000UL
#define FIXADDR_END     0xfff00000UL
                SHOW_DELTA_M((TYPECST)FIXADDR_START, (TYPECST)FIXADDR_END));
#else
#include <asm/fixmap.h>
                SHOW_DELTA_M((TYPECST)FIXADDR_START, (TYPECST)FIXADDR_START+FIXADDR_SIZE));
#endif

	/* kernel module region
	 * It's high in the kernel segment for typical 64-bit systems, but the
	 * other way around on 32-bit; so our 'show in descending order' thing
	 * won't really work here!
	 */
	pr_info(
	"|module region:      "
	" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " MB]\n",
		SHOW_DELTA_M((TYPECST)MODULES_VADDR, (TYPECST)MODULES_END));

#ifdef CONFIG_KASAN  // KASAN region: Kernel Address SANitizer
	pr_info(
	"|KASAN shadow:       "
	" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " GB]\n",
		SHOW_DELTA_G((TYPECST)KASAN_SHADOW_START, (TYPECST)KASAN_SHADOW_END));
#endif
	pr_info(
	"|vmalloc region:     "
	" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " MB = " FMTSPC_DEC " GB]"
	"\n"
	"|lowmem region:      "
	" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " MB = " FMTSPC_DEC " GB]"
	" (PAGE_OFFSET to RAM-size)\n"
	ELLPS,
		SHOW_DELTA_MG((TYPECST)VMALLOC_START, (TYPECST)VMALLOC_END),
		SHOW_DELTA_MG((TYPECST)PAGE_OFFSET, (TYPECST)high_memory));

#ifdef CONFIG_HIGHMEM  // zone HIGHMEM may be present on 32-bit systems with more RAM
	pr_info(
	"|HIGHMEM(pkmap) region: "
	" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " GB]\n",
		SHOW_DELTA_M((TYPECST)PKMAP_BASE,
			     (TYPECST)(PKMAP_BASE)+(LAST_PKMAP*PAGE_SIZE)));
#endif
	/*
	 * Symbols for kernel:
	 *   text begin/end (_text/_etext)
	 *   init begin/end (__init_begin, __init_end)
	 *   data begin/end (_sdata, _edata)
	 *   bss begin/end (__bss_start, __bss_stop)
	 * are only defined *within* (in-tree) and aren't available for modules
	 */
}

static int __init kernel_seg_init(void)
{
	pr_debug("%s: inserted\n", OURMODNAME);

	/* Display some minimal system info
	 * Note: this function is within our kernel 'library' here:
	 *  ../../llkd_klib.c
	 * Hence, we must arrange to link it in (see the Makefile)
	 */
	llkd_minsysinfo();
	show_kernelseg_info();

	if (show_uservas)
		show_userspace_info();
	else
		pr_info("%s: skipping show userspace...\n", OURMODNAME);

	return 0;	/* success */
}

static void __exit kernel_seg_exit(void)
{
	pr_debug("%s: removed\n", OURMODNAME);
}

module_init(kernel_seg_init);
module_exit(kernel_seg_exit);
