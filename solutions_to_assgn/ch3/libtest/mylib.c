/*
 * solutions_to_assgn/ch3/libtest/mylib.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch 3: Writing your First Kernel Module- LKMs Part 2
 ****************************************************************
 * Brief Description: Assignment:
3.1 : libtest: Write a kernel module called mycaller.c; it must invoke a
'library' routine called 'product' that lives within another 'C' file (mylib.c).
 int product(int a, int b); returns (a*b)
Link them into a single kernel module called libtest.ko and verify that the
caller can indeed correctly invoke code from the 'library'.

 * This is the mylib.c - the 'library' - code.
 */
#include <linux/init.h>
#include <linux/module.h>

int product(int a, int b)
{
	pr_debug("In %s()\n", __func__);
	return (a*b);
}
