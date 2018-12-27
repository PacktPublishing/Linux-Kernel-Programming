/*
 * ch2/modstacking/core_lkm.c
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
 * This kernel module - core_lkm - is part of the 'modstacking' POC project:
 *    user_lkm
 *        |
 *    core_lkm
 * The user_lkm kernel module calls an (exported) function that resides 
 * in the core_lkm kernel module.
 *
 * For details, please refer the book, Ch 2.
 */
#include <linux/init.h>
#include <linux/module.h>

#define MODNAME   "core_lkm"
#define THE_ONE   0xfedface
MODULE_LICENSE("Dual MIT/GPL");

static int exp_int = 200;
EXPORT_SYMBOL_GPL(exp_int);

/* Functions to be called from other LKMs */
#if(BITS_PER_LONG == 32)
static u32 get_skey(int p)
#else   // 64-bit
static u64 get_skey(int p)
#endif
{
#if(BITS_PER_LONG == 32)
	u32 secret = 0x567def;
#else   // 64-bit
	u64 secret = 0x123abc567def;
#endif
	if (p == THE_ONE)
		return secret;
	return 0;
}
EXPORT_SYMBOL(get_skey);

static int xfunc(void)
{
	pr_info("%s: %s:%s():%d: I've been called\n",
		MODNAME, __FILE__, __FUNCTION__, __LINE__);
	return exp_int;
}
EXPORT_SYMBOL(xfunc);

static int __init core_init(void)
{
	pr_info("%s: successfully inserted\n", MODNAME);
	return 0;
}

static void __exit core_exit(void)
{
	pr_info("%s: bids you adieu\n", MODNAME);
}

module_init(core_init);
module_exit(core_exit);
