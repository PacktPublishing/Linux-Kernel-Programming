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
 * From: Ch 6: Kernel and Memory Management Internals -Essentials Part 2
 ****************************************************************
 * Brief Description:
 * A kernel module to show us stuff regarding the layout of the kernel segment;
 * the kernel VAS (Virtual Address Space). In effect, showing us a simple memory
 * map of the kernel. Works on both 32 and 64-bit systems of differing
 * architectures (CPUs).
 * Optionally also displays key segment of the user VAS if the module parameter
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
#include "../../convenient.h"

#define OURMODNAME   "kernel_seg"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LLKD book:ch4/kernel_seg: display some kernel space details");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/* Module parameters */
static int show_uservas;
module_param(show_uservas, int, 0660);
MODULE_PARM_DESC(show_uservas,
        "Show some userspace VAS details; 0 = no (default), 1 = show");

#if(BITS_PER_LONG == 32)
	#define FMTSPC		"%08x"
	#define FMTSPC_DEC	"%08d"
	#define TYPECST		unsigned int
	#define MY_PATTERN1     0xdeadface
	#define MY_PATTERN2     0xffeeddcc
#elif(BITS_PER_LONG == 64)
	#define FMTSPC		"%016lx"
	#define FMTSPC_DEC	"%9ld"
	#define TYPECST	        unsigned long
	#define MY_PATTERN1     0xdeadfacedeadface
	#define MY_PATTERN2     0xffeeddccbbaa9988
#endif

/* From the ch6/min_sysinfo LKM */
void llkd_sysinfo2(void);

static int __init kernel_seg_init(void)
{
	pr_debug("%s: inserted\n", OURMODNAME);

	/* Display some minimal system info
	 * Note- this function is exported via the min_sysinfo.ko LKM;
	 * hence we have a dependency; that LKM must be loaded up first.
	 */
	llkd_sysinfo2();

	pr_info(
    "\nSome Kernel Details [by decreasing address]\n"
	"+-------------------------------------------------------------+\n"
	"|                           [ . . . ]                         |\n"
	"|Module space:       "
	" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " MB]\n",
		(TYPECST)MODULES_VADDR, (TYPECST)MODULES_END,
		(TYPECST)((MODULES_END-MODULES_VADDR)/(1024*1024)));

#ifdef CONFIG_KASAN  // Kernel Address SANitizer
	pr_info(
	"|KASAN shadow:       "
	" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " GB]\n",
	(TYPECST)KASAN_SHADOW_START, (TYPECST)KASAN_SHADOW_END,
	(TYPECST)((KASAN_SHADOW_END-KASAN_SHADOW_START)/(1024*1024*1024)));
#endif

	pr_info(
	"|Vmalloc region:     "
	" 0x" FMTSPC " - 0x" FMTSPC " | [" FMTSPC_DEC " MB = " FMTSPC_DEC " GB]" "\n"
	"[Lowmem:PAGE_OFFSET = 0x" FMTSPC " : start of all phy mapped RAM (here to RAM-size)]\n"
	"|                           [ . . . ]                         |\n",
		(TYPECST)VMALLOC_START, (TYPECST)VMALLOC_END,
		(TYPECST)((VMALLOC_END-VMALLOC_START)/(1024*1024)), 
		(TYPECST)((VMALLOC_END-VMALLOC_START)/(1024*1024*1024)),
		(TYPECST)PAGE_OFFSET);

	if (!show_uservas) {
		pr_info("%s: skipping show userspace...\n", OURMODNAME);
		return 0;
	}

	pr_info (
	"+------------ Above, kernel-space; Below, userspace ----------+\n"
	"|                           [ . . . ]                         |\n"
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
	"|                           [ . . . ]                         |\n"
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
		(TYPECST)TASK_SIZE, (TYPECST)TASK_SIZE/(1024*1024*1024),
		current->mm->map_count);
		//current->tgid, current->pid, current->comm);
	PRINT_CTX();       // see it in the convenient.h header

	return 0;
}

static void __exit kernel_seg_exit(void)
{
	pr_debug("%s: removed\n", OURMODNAME);
}

module_init(kernel_seg_init);
module_exit(kernel_seg_exit);
