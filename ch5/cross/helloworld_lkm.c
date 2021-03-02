/*
 * ch5/cross/helloworld_lkm.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming
 *
 * From: Ch 5 : Writing your First Kernel Module - LKMs Part 2
 ****************************************************************
 * Brief Description:
 * Our very first kernel module, the 'Hello, world' of course! The
 * idea being to explain the essentials of the Linux kernel's LKM
 * framework.
 * This time, we're attempting to cross compile and run it on the
 * Raspberry Pi SBC! This does not affect the code in any major way, of course,
 * but *does* affect the Makefile. Check out the new and enhanced Makefile
 * carefully.
 *
 * For details, please refer the book, Ch 5.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKP book:ch5/cross: hello, world, our first Raspberry Pi LKM");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int __init hello_LLKD_init(void)
{
	pr_info("Hello, Raspberry Pi world\n");
	return 0;		/* success */
}

static void __exit hello_LLKD_exit(void)
{
	pr_info("Goodbye, Raspberry Pi world\n");
}

module_init(hello_LLKD_init);
module_exit(hello_LLKD_exit);
