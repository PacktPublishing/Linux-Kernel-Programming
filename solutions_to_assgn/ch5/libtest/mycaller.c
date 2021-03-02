/*
 * solutions_to_assgn/ch3/libtest/mycaller.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming
 *
 * From: Ch 5: Writing your First Kernel Module- LKMs Part 2
 ****************************************************************
 * Brief Description: Assignment:
 * 5.1 : libtest: 
 * Write a kernel module called mycaller.c. It must invoke a library routine
 * called 'product' that lives within another C file (mylib.c), whose signature
 * is: int product(int a, int b); and will return the value (a*b).
 * Link them into a single kernel module called libtest.ko and verify that the
 * caller can indeed correctly invoke code from the 'library'.
 * 
 * This is the mycaller.c code.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define OURMODNAME   "libtest"

MODULE_AUTHOR("<insert your name here>");
MODULE_DESCRIPTION("LKP book assignment : ch5/libtest");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

int prodint(int, int);

static int __init mycaller_init(void)
{
	int a = 500, b = 900, res;

	pr_info("inserted\n");

	pr_info("calling product(%d,%d) now...\n", a, b);
	res = prodint(a, b);
	pr_info(" res = %d\n", res);

	a = 10000; b = 250000;
	pr_info("calling product(%d,%d) now...\n", a, b);
	res = prodint(a, b);
	pr_info(" res = %d\n", res);

	return 0;		/* success */
}

static void __exit mycaller_exit(void)
{
	pr_info("removed\n");
}

module_init(mycaller_init);
module_exit(mycaller_exit);
