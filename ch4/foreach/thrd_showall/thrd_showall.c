/*
 * thrd_showall.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch 4 : Memory Allocation for Module Authors
 ****************************************************************
 * Brief Description:
 *
 * For details, please refer the book, Ch 4.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
#include <linux/sched/signal.h>
#endif

#define OURMODNAME   "thrd_showall"

MODULE_AUTHOR("<insert your name here>");
MODULE_DESCRIPTION("LKDC book:ch4/foreach/thrd_showall:"
		   " demo to display all threads by iterating over the task list");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int showthrds(void)
{
	struct task_struct *g, *t;	// 'g' : process ptr; 't': thread ptr
	int nr_thrds = 1, total = 0;
	char buf[256], tmp[128];

	pr_info
	    ("----------------------------------------------------------------------------\n"
	     "    TGID   PID               Thread Name        # Threads\n"
	     "----------------------------------------------------------------------------\n");

	do_each_thread(g, t) {
		task_lock(t);

		snprintf(buf, 256, "%6d %6d ", g->tgid, t->pid);
		if (!g->mm) {	// kernel thread
			snprintf(tmp, 128, "[%30s]", t->comm);
		} else {
			snprintf(tmp, 128, " %30s ", t->comm);
		}
		strncat(buf, tmp, 128);

		/* Is this the "main" thread of a multithreaded process?
		 * (we check by seeing if (a) it's a userspace thread, 
		 * (b) it's TGID == it's PID, and (c), there are >1 threads in
		 * the process.
		 * If so, display the number of threads in the overall process
		 * to the right..
		 */
		nr_thrds = get_nr_threads(g);
		if (g->mm && (g->tgid == t->pid) && (nr_thrds > 1)) {
			snprintf(tmp, 128, "    %4d", nr_thrds);
			strncat(buf, tmp, 128);
		}

		snprintf(tmp, 2, "\n");
		strncat(buf, tmp, 2);
		pr_info("%s", buf);

		total++;
		memset(buf, 0, sizeof(buf));
		memset(tmp, 0, sizeof(tmp));
		task_unlock(t);
	} while_each_thread(g, t);

	return total;
}

static int __init thrd_showall_init(void)
{
	int total;

	pr_debug("%s: inserted\n", OURMODNAME);
	total = showthrds();
	pr_info("%s: total # of threads on the system: %d\n",
		OURMODNAME, total);

	return 0;		/* success */
}

static void __exit thrd_showall_exit(void)
{
	pr_debug("%s: removed\n", OURMODNAME);
}

module_init(thrd_showall_init);
module_exit(thrd_showall_exit);
