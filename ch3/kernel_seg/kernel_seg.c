/*
 * ch3/kernel_seg/kernel_seg.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch 3: Kernel and Memory Management Internals -Essentials
 ****************************************************************
 * Brief Description:
 * A kernel module to show us abot about the layout of the kernel segment;
 * kernel VAS (Virtual Address Space). In effect, showing us a simple memory
 * map of the kernel. Works on both 32 and 64-bit systems.
 *
 * For details, please refer the book, Ch 3.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/pgtable.h>
#include "../../convenient.h"

#define OURMODNAME   "kernel_seg"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKDC book:ch3/kernel_seg: display some kernel space details");
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
	#define FMTSPC_DEC	"%ld"
	#define TYPECST	        unsigned long
	#define MY_PATTERN1     0xdeadfacedeadface
	#define MY_PATTERN2     0xffeeddccbbaa9988
#endif

/* From the ch2/min_sysinfo LKM */
void lkdc_sysinfo2(void);

static int __init kernel_seg_init(void)
{
	pr_debug("%s: inserted\n", OURMODNAME);

	/* Display some minimal system info
	 * Note- this function is exported via the min_sysinfo.ko LKM;
	 * hence we have a dependency; that LKM must be loaded up first.
	 */
	lkdc_sysinfo2();

	pr_info(
    "\nSome Kernel Details [sorted by decreasing address] -------------------\n"
#ifdef CONFIG_X86
	" FIXADDR_START       = 0x" FMTSPC "\n"
#if(BITS_PER_LONG == 32)
	" PKMAP_BASE          = 0x" FMTSPC "\n"
#endif
#endif
	" MODULES_END         = 0x" FMTSPC "\n"
	" MODULES_VADDR       = 0x" FMTSPC " [modules range: " FMTSPC_DEC " MB]\n"
#ifdef CONFIG_X86
	" CPU_ENTRY_AREA_BASE = 0x" FMTSPC "\n"
	" VMEMMAP_START       = 0x" FMTSPC "\n"
#endif
	" VMALLOC_END         = 0x" FMTSPC "\n"
	" VMALLOC_START       = 0x" FMTSPC " [vmalloc range: " FMTSPC_DEC " MB =" FMTSPC_DEC " GB]" "\n"
	" PAGE_OFFSET         = 0x" FMTSPC " [lowmem region: start of all phy mapped RAM (here to RAM-size)]\n",
#ifdef CONFIG_X86
		(TYPECST)FIXADDR_START,
#if(BITS_PER_LONG == 32)
		(TYPECST)PKMAP_BASE,
#endif
#endif
		(TYPECST)MODULES_END, (TYPECST)MODULES_VADDR,
		 (TYPECST)((MODULES_END-MODULES_VADDR)/(1024*1024)),
#ifdef CONFIG_X86
		(TYPECST)CPU_ENTRY_AREA_BASE,
		(TYPECST)VMEMMAP_START,
#endif
		(TYPECST)VMALLOC_END, (TYPECST)VMALLOC_START,
		 (TYPECST)((VMALLOC_END-VMALLOC_START)/(1024*1024)), 
		 (TYPECST)((VMALLOC_END-VMALLOC_START)/(1024*1024*1024)),
		(TYPECST)PAGE_OFFSET);

#ifdef CONFIG_KASAN
	pr_info("\nKASAN_SHADOW_START = 0x" FMTSPC " KASAN_SHADOW_END = 0x" FMTSPC "\n",
		(TYPECST)KASAN_SHADOW_START, (TYPECST)KASAN_SHADOW_END);
#endif
	if (!show_uservas) {
		pr_info("%s: skipping show userspace...\n", OURMODNAME);
		return 0;
	}

	pr_info("%s: Process Userspace - some details:\n", OURMODNAME);
	PRINT_CTX();       // see it in the convenient.h header
	pr_info (
    "\nProcess Usermode VAS [sorted by decreasing address] ------------------\n"
	" [TASK_SIZE         = 0x" FMTSPC " size of userland]\n"
	" [Statistics wrt 'current' thread TGID=%d PID=%d name=%s]:\n"
	"        env_end     = 0x" FMTSPC "\n"
	"        env_start   = 0x" FMTSPC "\n"
	"        arg_end     = 0x" FMTSPC "\n"
	"        arg_start   = 0x" FMTSPC "\n"
	"        start_stack = 0x" FMTSPC "\n"
	"        curr brk    = 0x" FMTSPC "\n" 
	"        start_brk   = 0x" FMTSPC "\n"
	"        end_data    = 0x" FMTSPC "\n"
	"        start_data  = 0x" FMTSPC "\n" 
	"        end_code    = 0x" FMTSPC "\n"
	"        start_code  = 0x" FMTSPC "\n"
	" [# memory regions (VMAs) = %d]\n",
		(TYPECST)TASK_SIZE,
		current->tgid, current->pid, current->comm,
		(TYPECST)current->mm->env_end,
		(TYPECST)current->mm->env_start,
		(TYPECST)current->mm->arg_end, 
		(TYPECST)current->mm->arg_start,
		(TYPECST)current->mm->start_stack,
		(TYPECST)current->mm->brk,
		(TYPECST)current->mm->start_brk,
		(TYPECST)current->mm->end_data,
		(TYPECST)current->mm->start_data,
		(TYPECST)current->mm->end_code,
		(TYPECST)current->mm->start_code,
		current->mm->map_count);

	return 0;
}

static void __exit kernel_seg_exit(void)
{
	pr_debug("%s: removed\n", OURMODNAME);
}

module_init(kernel_seg_init);
module_exit(kernel_seg_exit);
