/*
 * ch5/lkm_template/lkm_template.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming
 *
 * From: Ch 5 : Writing Your First Kernel Module LKMs, Part 2
 ****************************************************************
 * Brief Description:
 * A very simple 'template' of sorts for a Loadable Kernel Module(s) (LKM).
 * Do take the time and trouble to study it via the book's Ch 5; don't ignore
 * it's 'better' Makefile !
 *
 * For details, please refer the book, Ch 5.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define OURMODNAME   "lkm_template"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("a simple LKM template; do refer to the (better) Makefile as well");
MODULE_LICENSE("Dual MIT/GPL");	// or whatever
MODULE_VERSION("0.1");

static int __init lkm_template_init(void)
{
	pr_info("inserted\n");
	return 0;		/* success */
}

static void __exit lkm_template_exit(void)
{
	pr_info("removed\n");
}

module_init(lkm_template_init);
module_exit(lkm_template_exit);
