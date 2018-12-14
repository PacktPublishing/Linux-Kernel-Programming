/*
 * ch2/hello.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *    https://github.com/PacktPublishing/--fill-in--
 *
 * From: Ch 2: Writing your First Kernel Module-the LKM Framework
 ****************************************************************
 * Brief Description:
 *
 *
 * For details, please refer the book, Ch 2.
 */
#include <linux/init.h>
#include <linux/module.h>

MODULE_AUTHOR("<insert your name here>");
MODULE_DESCRIPTION("LKDC book:ch2: hello, world, our first LKM");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int __init hello_lkdc_init(void)
{
	printk(KERN_INFO "Hello, world\n");
	return 0;		/* success */
}

static void __exit hello_lkdc_exit(void)
{
	printk(KERN_INFO "Goodbye, world\n");
}

module_init(hello_lkdc_init);
module_exit(hello_lkdc_exit);
