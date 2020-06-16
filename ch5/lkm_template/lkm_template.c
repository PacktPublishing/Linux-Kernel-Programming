/*
 * ch5/lkm_template/lkm_template.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 5 : Writing Your First Kernel Module LKMs, Part 2
 ****************************************************************
 * Brief Description:
 * A very simple Loadable Kernel Module (LKM) 'template' of sorts; the good
 * stuff's actually within it's Makefile.
 *
 * For details, please refer the book, Ch 5.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define OURMODNAME   "lkm_template"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("a simple LKM template; do refer to the Makefile");
MODULE_LICENSE("Dual MIT/GPL");	// or whatever
MODULE_VERSION("0.1");

static int __init lkm_template_init(void)
{
	pr_info("%s: inserted\n", OURMODNAME);
	return 0;		/* success */
}

static void __exit lkm_template_exit(void)
{
	pr_info("%s: removed\n", OURMODNAME);
}

module_init(lkm_template_init);
module_exit(lkm_template_exit);
