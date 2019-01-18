/*
 * ch4/current_affairs/current_affairs.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch 4: Memory Allocation for Module Authors
 ****************************************************************
 * Brief Description:
 *
 * For details, please refer the book, Ch .
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>  /* current */

#define OURMODNAME   "current_affairs"

MODULE_AUTHOR("<insert your name here>");
MODULE_DESCRIPTION("LKDC book:ch/: hello, world");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static inline void show_ctx(char *nm)
{
	pr_info("%s:%s():%d ", nm, __func__, __LINE__);
	if (likely(!in_interrupt())) {
		pr_info(
		"%s: in process context ::\n"
		" PID         : %6d\n"
		" TGID        : %6d\n"
		" name        : %15s\n"
		" current (ptr to our process context's task_struct) : 0x%pK\n"
		" stack start : 0x%16pK\n",
		nm, current->pid, current->tgid, current->comm,
		current, current->stack);
	} else
		pr_alert("%s: in interrupt context [Should NOT Happen here!]\n",
			nm);
}

static int __init current_affairs_init(void)
{
	pr_debug("%s: inserted\n", OURMODNAME);
	show_ctx(OURMODNAME);
	return 0;		/* success */
}

static void __exit current_affairs_exit(void)
{
	show_ctx(OURMODNAME);
	pr_debug("%s: removed\n", OURMODNAME);
}

module_init(current_affairs_init);
module_exit(current_affairs_exit);
