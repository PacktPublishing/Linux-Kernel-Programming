/*
 * ch2/cross/hello.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch 2: Writing your First Kernel Module-the LKM Framework
 ****************************************************************
 * Brief Description:
 * Our very first kernel module, the 'Hello, world' of course! The
 * idea being to explain the essentials of the Linux kernel's LKM
 * framework.
 * This time, we're attempting to cross compile and run it on the
 * Raspberry Pi SBC! This does not affect the code in any major way, of course,
 * but *does* affect the Makefile. Check out the new and enhanced Makefile
 * carefully. Also notice how we emit a printk based on the CPU we're running
 * upon.
 *
 * For details, please refer the book, Ch 2.
 */
#include <linux/init.h>
#include <linux/module.h>

MODULE_AUTHOR("<insert your name here>");
MODULE_DESCRIPTION("LKDC book:ch2/cross: hello, world, our first Raspberry Pi LKM");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int __init hello_lkdc_init(void)
{
	char tmp[128], msg[256];

	memset(msg, 0, strlen(msg));
	memset(tmp, 0, strlen(tmp));

	strncpy(msg, "Hello, ", 8);
	/* Strictly speaking, all this #if... is considered ugly and should be
	   isolated as far as is possible */
#ifdef CONFIG_X86
#if(BITS_PER_LONG == 32)
	strncat(msg, "x86-32", 7);
#else
	strncat(msg, "x86_64", 7);
#endif
#endif
#ifdef CONFIG_ARM
	strncat(msg, "ARM-32", 7);
#endif
	strncat(msg, " world\n", 8);
	printk(KERN_INFO "%s", msg);

	return 0;		/* success */
}

static void __exit hello_lkdc_exit(void)
{
	printk(KERN_INFO "Goodbye, world\n");
}

module_init(hello_lkdc_init);
module_exit(hello_lkdc_exit);
