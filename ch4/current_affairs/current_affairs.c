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
 * From: Ch 4: 
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

static int __init current_affairs_init(void)
{
	pr_debug("%s: inserted\n", OURMODNAME);
	pr_info("current (ptr to our process context's task_struct) : 0x%pK\n",
		current);
	pr_info(" PID         : %6d\n"
		" TGID        : %6d\n"
		" stack start : 0x%16pK\n"
		" name        : %15s\n",
		current->pid, current->tgid,
		current->stack, current->comm);
	return 0;		/* success */
}

static void __exit current_affairs_exit(void)
{
	pr_debug("%s: removed\n", OURMODNAME);
}

module_init(current_affairs_init);
module_exit(current_affairs_exit);
