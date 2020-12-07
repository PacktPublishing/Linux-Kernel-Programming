/*
 * solutions_to_assgn/ch15/workq_simple2/workq_simple2.c
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
 * Exercise question:
 *
 * workq_simple2:
 * Build upon the previous assignment (workq_simple2) to execute two work tasks
 * plus one more task (from the init code path); this one (the third one) should
 * be delayed; the amount of time to delay by should be passed as a module
 * parameter named work_delay_ms (in milliseconds; default should be 500 ms).
 *
 * For details, please refer the book, Ch 15.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include "../../convenient.h"

#define OURMODNAME   "workq_simple2"
#define INITIAL_VALUE	3

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("solution: ch15:a LKM to demo a workqueue with 2 functions");
MODULE_LICENSE("Dual MIT/GPL");	// or whatever
MODULE_VERSION("0.1");

static int work_delay_ms = 500;
module_param(work_delay_ms, uint, 0440);
MODULE_PARM_DESC(work_delay_ms,
"Set to the delay (in ms) after which the delayed work task runs (default=500)");

static struct st_ctx {
	struct work_struct work1;
	struct work_struct work2;
	struct timer_list tmr;
	int data;
	u64 t1s, t1e, t2s, t2e; // s=start, e=end :-p
	//--- for the delayed work task
	u64 td1, td2;
	struct delayed_work dwork;
} ctx;
static unsigned long exp_ms = 420;

/*
 * ding() - our timer's callback function!
 */
static void ding(struct timer_list *timer)
{
	struct st_ctx *priv = from_timer(priv, timer, tmr);

	pr_debug("timed out... data=%d\n", priv->data--);
	PRINT_CTX();

	/* until countdown done, fire it again! */
	if (priv->data)
		mod_timer(&priv->tmr, jiffies + msecs_to_jiffies(exp_ms));

	/* Now 'schedule' our work queue function to run */
	if (!schedule_work(&priv->work1))
		pr_notice("work1: our work's already on the kernel-global workqueue!\n");
	priv->t1s = ktime_get_real_ns();
	if (!schedule_work(&priv->work2))
		pr_notice("work2: our work's already on the kernel-global workqueue!\n");
	priv->t2s = ktime_get_real_ns();
}

/*
 * work_func1() - our first workqueue callback function!
 */
static void work_func1(struct work_struct *work)
{
	struct st_ctx *priv = container_of(work, struct st_ctx, work1);

	priv->t1e = ktime_get_real_ns();
	pr_info("In our workq #1 function: data=%d\n", priv->data);
	PRINT_CTX();
	SHOW_DELTA(priv->t1e, priv->t1s);
}
/*
 * work_func2() - our second workqueue callback function!
 */
static void work_func2(struct work_struct *work)
{
	struct st_ctx *priv = container_of(work, struct st_ctx, work2);

	priv->t2e = ktime_get_real_ns();
	pr_info("In our workq #2 function: data=%d\n", priv->data);
	PRINT_CTX();
	SHOW_DELTA(priv->t2e, priv->t2s);
}
/*
 * work_func_delayed() - our third, delayed workqueue callback function!
 */
static void work_func_delayed(struct work_struct *work)
{
	struct st_ctx *priv = container_of(work, struct st_ctx, dwork.work);
		/* check out the third param! it's the pointer to the work_struct within
		 * the delayed_work structure
		 */

	priv->td2 = ktime_get_real_ns();
	pr_info("In our workq delayed #3 function: data=%d\n", priv->data);
	PRINT_CTX();
	SHOW_DELTA(priv->td2, priv->td1);
}

static int __init workq_delayed_init(void)
{
	memset(&ctx, 0, sizeof(struct st_ctx));
	ctx.data = INITIAL_VALUE;

	/* Initialize our workqueues */
	INIT_WORK(&ctx.work1, work_func1);
	INIT_WORK(&ctx.work2, work_func2);

	/* Initialize our kernel timer */
	ctx.tmr.expires = jiffies + msecs_to_jiffies(exp_ms);
	ctx.tmr.flags = 0;
	timer_setup(&ctx.tmr, ding, 0);

	pr_info("Work queue initialized, timer set to expire in %ld ms\n", exp_ms);
	add_timer(&ctx.tmr); /* Arm it; lets get going! */

	INIT_DELAYED_WORK(&ctx.dwork, work_func_delayed);
	if (!schedule_delayed_work(&ctx.dwork, msecs_to_jiffies(work_delay_ms)))
        pr_notice("dwork: our work's already on the kernel-global workqueue!\n");
	else
		pr_info("Delayed work task scheduled to execute in %u ms\n", work_delay_ms);
    ctx.td1 = ktime_get_real_ns();

	return 0;		/* success */
}

static void __exit workq_delayed_exit(void)
{
	// Wait for any pending work (queue) to finish
	if (cancel_work_sync(&ctx.work1))
		pr_info("work1: yes, there was indeed some pending work; now done...\n");
	if (cancel_work_sync(&ctx.work2))
		pr_info("work2: yes, there was indeed some pending work; now done...\n");
	/* flush_scheduled_work(); Alternative, but harder to use correctly, as well
	 * as overkill; cancel_work_[delayed_]sync() is simpler.. */
	if (cancel_delayed_work_sync(&ctx.dwork))
        pr_info("dwork: yes, there was indeed some pending work; now done...\n");

	// Wait for possible timeouts to complete... and then delete the timer
	del_timer_sync(&ctx.tmr);
	pr_info("removed\n");
}

module_init(workq_delayed_init);
module_exit(workq_delayed_exit);
