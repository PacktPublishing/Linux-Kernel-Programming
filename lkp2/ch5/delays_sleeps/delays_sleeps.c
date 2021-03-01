/*
 * ch15/delays_sleeps/delays_sleeps.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 15 : Kernel timers, workqueues and more
 ****************************************************************
 * Brief Description:
 *
 * For details, please refer the book, Ch 15.
 */
//#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__
// disabled here, as the kernel log o/p becomes too many cols...

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ktime.h>		// ktime_get_*() routines

#define OURMODNAME   "delays_sleeps"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("a simple LKM to demo delays and sleeps in the kernel");
MODULE_LICENSE("Dual MIT/GPL");	// or whatever
MODULE_VERSION("0.1");

/*
 * DILLY_DALLY() macro:
 * Runs the code @run_this while measuring the time it takes; prints the string
 * @code_str to the kernel log along with the actual time taken (in ns, us
 * and ms).
 * Macro inspired by the book 'Linux Device Drivers Cookbook', PacktPub.
 */
#define DILLY_DALLY(code_str, run_this) do {	\
	u64 t1, t2;									\
	t1 = ktime_get_real_ns();					\
	run_this;									\
	t2 = ktime_get_real_ns();					\
	pr_info(code_str "-> actual: %11llu ns = %7llu us = %4llu ms\n", \
		(t2-t1), (t2-t1)/1000, (t2-t1)/1000000);\
} while (0)

static int __init delays_sleeps_init(void)
{
	pr_info("%s: inserted\n", OURMODNAME);

	/* Atomic busy-loops, no sleep! */
	pr_info("\n1. *delay() functions (atomic, in a delay loop):\n");
	DILLY_DALLY("ndelay() for         10 ns", ndelay(10));
	/* udelay() is the preferred interface */
	DILLY_DALLY("udelay() for     10,000 ns", udelay(10));
	DILLY_DALLY("mdelay() for 10,000,000 ns", mdelay(10));

	/* Non-atomic blocking APIs; causes schedule() to be invoked */
	pr_info("\n2. *sleep() functions (process ctx, sleeps/schedule()'s out):\n");
	/* usleep_range(): HRT-based, 'flexible'; for approx range [10us - 20ms] */
	DILLY_DALLY("usleep_range(10,10) for 10,000 ns", usleep_range(10, 10));
	/* msleep(): jiffies/legacy-based; for longer sleeps (> 10ms) */
	DILLY_DALLY("msleep(10) for      10,000,000 ns", msleep(10));
	DILLY_DALLY("msleep_interruptible(10)         ", msleep_interruptible(10));
	/* ssleep() is a wrapper over msleep():  = msleep(ms*1000); */
	DILLY_DALLY("ssleep(1)                        ", ssleep(1));

	return 0;		/* success */
}

static void __exit delays_sleeps_exit(void)
{
	pr_info("%s: removed\n", OURMODNAME);
}

module_init(delays_sleeps_init);
module_exit(delays_sleeps_exit);
