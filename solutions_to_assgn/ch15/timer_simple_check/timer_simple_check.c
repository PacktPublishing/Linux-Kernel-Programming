/*
 * solutions_to_assgn/ch15/timer_simple_check/timer_simple_check.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 15 : Timers, kernel threads and more
 ****************************************************************
 * Questions:
 * timer_simple_check: enhance the timer_simple kernel module to check the
 * amount of time elapsed between setting up a timeout and it actually
 * being serviced.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/ktime.h>
#include "../../convenient.h"

#define OURMODNAME   "timer_simple_check"
#define INITIAL_VALUE	3

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("sample solution: solutions_to_assgn/ch15/timer_simple_check/");
MODULE_LICENSE("Dual MIT/GPL");	// or whatever
MODULE_VERSION("0.1");

static struct st_ctx {
	struct timer_list tmr;
	int data;
} ctx;
static unsigned long exp_ms = 420;
static u64 t1, t2;

static void ding(struct timer_list *timer)
{
	struct st_ctx *priv = from_timer(priv, timer, tmr);

	t2 = ktime_get_real_ns();

	pr_debug("timed out... data=%d\n", priv->data--);
	/*
	 * Using the time_after() is a good idea; a validity check ensuring that
	 * the t2 timestamp is actually 'later' (greater) than the 't1' timestamp
	 * See the comment above here: include/linux/jiffies.h:#define time_after(a,b)
	 */
	if (time_after((unsigned long)t2, (unsigned long)t1))
		pr_info("-> actual: %11llu ns = %7llu us = %4llu ms\n",
				(t2-t1), (t2-t1)/1000, (t2-t1)/1000000);
	PRINT_CTX();

	/* until countdown done, fire it again! */
	if (priv->data) {
		mod_timer(&priv->tmr, jiffies + msecs_to_jiffies(exp_ms));
		t1 = t2;
	}
}

static int __init timer_simple_check_init(void)
{
	ctx.data = INITIAL_VALUE;
	ctx.tmr.expires = jiffies + msecs_to_jiffies(exp_ms);
	ctx.tmr.flags = 0;
	timer_setup(&ctx.tmr, ding, 0);

	pr_info("timer set to expire in %ld ms\n", exp_ms);
	t1 = ktime_get_real_ns();
	add_timer(&ctx.tmr); /* Arm it; lets get going! */

	return 0;		/* success */
}

static void __exit timer_simple_check_exit(void)
{
	pr_debug("wait for possible timeouts to complete...\n");
	del_timer_sync(&ctx.tmr);
	pr_info("removed\n");
}

module_init(timer_simple_check_init);
module_exit(timer_simple_check_exit);
