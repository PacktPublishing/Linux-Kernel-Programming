/*
 * ch2/modstacking/user_lkm.c
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
 * This kernel module - user_lkm - is part of the 'modstacking' POC project:
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

#define MODNAME     "user_lkm"
#if 1
MODULE_LICENSE("Dual MIT/GPL");
#else
MODULE_LICENSE("MIT");
#endif

extern void lkdc_sysinfo(void);
extern int xfunc(void);
extern long get_skey(int);
extern int exp_int;

static int __init ontop_init(void)
{
#define THE_ONE   0xfedface
	u64 sk = get_skey(THE_ONE);

	pr_info("%s: successfully inserted\n", MODNAME);

	/* Call functions within the 'core' module */
	pr_debug("%s: Called xfunc(), ret = %d\n", MODNAME, xfunc());
	pr_debug("%s: Called get_skey(), ret = 0x%llx = %llu\n",
			MODNAME, sk, sk);
	pr_debug("%s: exp_int = %d\n", MODNAME, exp_int);
	lkdc_sysinfo();

	return 0;
}

static void __exit ontop_exit(void)
{
	pr_info("%s: bids you adieu\n", MODNAME);
}

module_init(ontop_init);
module_exit(ontop_exit);
