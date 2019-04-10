/*
 * dumpstack/dumpstack.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch : 
 ****************************************************************
 * Brief Description:
 *
 * For details, please refer the book, Ch .
 */
#include <linux/init.h>
#include <linux/module.h>

#define OURMODNAME   "dumpstack"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKDC book:ch11/: dump the stack demo");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int __init dumpstack_init(void)
{
	pr_debug("%s: inserted\n", OURMODNAME);
	return 0;		/* success */
}

static void __exit dumpstack_exit(void)
{
	dump_stack();
	pr_debug("%s: removed\n", OURMODNAME);
}

module_init(dumpstack_init);
module_exit(dumpstack_exit);
