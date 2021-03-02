/*
 * ch5/modstacking/user_lkm.c
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
 * Brief Description:
 * This kernel module - user_lkm - is part of the 'modstacking' POC project:
 *    user_lkm           [<--- this code]
 *        |
 *    core_lkm
 * The user_lkm kernel module calls an (exported) function that resides
 * in the core_lkm kernel module.
 *
 * For details, please refer the book, Ch 5.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

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
	pr_info("inserted\n");
	u64 sk = get_skey(THE_ONE);

	pr_debug("Called get_skey(), ret = 0x%llx = %llu\n", sk, sk);
	pr_debug("exp_int = %d\n", exp_int);
	llkd_sysinfo2();

	return 0;
}

static void __exit user_lkm_exit(void)
{
	pr_info("bids you adieu\n");
}

module_init(user_lkm_init);
module_exit(user_lkm_exit);
