/*
 * ch5/modstacking/user_lkm.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 5: Writing your First Kernel Module-the LKM Framework
 ****************************************************************
 * Brief Description:
 * This kernel module - user_lkm - is part of the 'modstacking' POC project:
 *    user_lkm
 *        |
 *    core_lkm
 * The user_lkm kernel module calls an (exported) function that resides 
 * in the core_lkm kernel module.
 *
 * For details, please refer the book, Ch 5.
 */
#include <linux/init.h>
#include <linux/module.h>

#define MODNAME     "user_lkm"
#if 1
MODULE_LICENSE("Dual MIT/GPL");
#else
MODULE_LICENSE("MIT");
#endif

extern void llkd_sysinfo2(void);
extern long get_skey(int);
extern int exp_int;

/* Call some functions within the 'core' module */
static int __init user_lkm_init(void)
{
#define THE_ONE   0xfedface
	u64 sk = get_skey(THE_ONE);

	pr_info("%s: inserted\n", MODNAME);
	pr_debug("%s: Called get_skey(), ret = 0x%llx = %llu\n",
			MODNAME, sk, sk);
	pr_debug("%s: exp_int = %d\n", MODNAME, exp_int);
	llkd_sysinfo2();

	return 0;
}

static void __exit user_lkm_exit(void)
{
	pr_info("%s: bids you adieu\n", MODNAME);
}

module_init(user_lkm_init);
module_exit(user_lkm_exit);
