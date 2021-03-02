/*
 * solutions_to_assgn/ch5/libtest/mylib.c
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
 * 5.1 : libtest: Write a kernel module called mycaller.c; it must invoke a
 * 'library' routine called 'product' that lives within another 'C' file (mylib.c).
 *  int product(int a, int b); returns (a*b)
 * Link them into a single kernel module called libtest.ko and verify that the
 * caller can indeed correctly invoke code from the 'library'.
 * 
 * This is the mylib.c - the 'library' - code.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#if 0
int product(int a, int b)
{
	pr_debug("In %s()\n", __func__);
	return (a*b);
}
#endif

/* prodint()
 * Multiply two signed integers and return the result.
 * The trivial implemnation is return (a*b);
 * However, it can overflow, we Must check!
 *
 * Src: Linux kernel: include/linux/overflow.h:__signed_mul_overflow macro
 * "Signed multiplication is rather hard. gcc always follows C99, so
 * division is truncated towards 0. This means that we can write the
 * overflow check like this:
 *
 * (a > 0 && (b > MAX/a || b < MIN/a)) ||
 * (a < -1 && (b > MIN/a || b < MAX/a) ||
 * (a == -1 && b == MIN)"
 * We merely imitate this.
 */
int prodint(int a, int b)
{
	if ((a > 0 && (b > INT_MAX/a || b < INT_MIN/a)) ||
 	    (a < -1 && (b > INT_MIN/a || b < INT_MAX/a))  ||
	    (a == -1 && b == INT_MIN)) {
		pr_warn("signed integer multiplication overflow detected!\n");
		return -1;
	}
	return (a*b);
}
