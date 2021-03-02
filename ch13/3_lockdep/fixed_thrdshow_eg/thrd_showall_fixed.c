/*
 * ch13/3_lockdep/fixed_thrdshow_eg/thrd_showall_fixed.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming
 *
 * From: Ch 13 : Kernel Synchronization, Part 2
 ****************************************************************
 * Brief Description:
 * This kernel module is based upon our earlier kernel module from Ch 6:
 *  ch6/foreach/thrd_showall/thrd_showall.c.
 * When we refactor it to use the get_task_comm() routine to
 * retrieve the name of the thread, it's buggy!
 * The bug turns out to be a recursive locking issue, *detected by lockdep*.
 *
 * Here, we fix it by first unlocking the relevant lock (struct task_struct
 * alloc_lock), then calling get_task_comm() and then re-locking it.
 *
 * For details, please refer the book, Ch 13.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>     /* current() */
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 10, 0)
#include <linux/sched/signal.h>
#endif

#define OURMODNAME   "thrd_showall_fixed"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKP book: ch13/3_lockdep/fixed_thrdshow_eg:"
" FIXED demo to display all threads by iterating over the task list");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int showthrds_fixed(void)
{
	struct task_struct *g, *t;  /* 'g' : process ptr; 't': thread ptr */
	int nr_thrds = 1, total = 0;
#define BUFMAX		256
#define TMPMAX		128
	char buf[BUFMAX], tmp[TMPMAX], tasknm[TASK_COMM_LEN];
	const char hdr[] =
"--------------------------------------------------------------------------------\n"
"    TGID   PID         current        stack-start      Thread Name   MT? # thrds\n"
"--------------------------------------------------------------------------------\n";

	pr_info("%s", hdr);
#if 0
	/* the tasklist_lock reader-writer spinlock for the task list 'should'
	 * be used here, but, it's not exported, hence unavailable to our
	 * kernel module */
	read_lock(&tasklist_lock);
#endif
	do_each_thread(g, t) {     /* 'g' : process ptr; 't': thread ptr */
		task_lock(t);  /*** task lock taken here! ***/

		snprintf(buf, BUFMAX-1, "%6d %6d ", g->tgid, t->pid);

		/* task_struct addr and kernel-mode stack addr */
		snprintf(tmp, TMPMAX-1, "  0x%016lx", (unsigned long)t);
		strncat(buf, tmp, TMPMAX);
		snprintf(tmp, TMPMAX-1, "  0x%016lx", (unsigned long)t->stack);
		strncat(buf, tmp, TMPMAX);

	/* In the 'buggy' ver of this code, LOCKDEP did catch a deadlock here !!
	 * (at the point that get_task_comm() was invoked).
	 * the reason: get_task_comm() attempts to take the very same lock
 	 * that we just took above: task_lock(t);  !! This is obvious self-deadlock...
	 * So, we fix it here by first unlocking it, calling get_task_comm(), and
	 * then re-locking it.
	 */
		task_unlock(t);
		get_task_comm(tasknm, t);
		task_lock(t);

		if (!g->mm)	// kernel thread
			snprintf(tmp, sizeof(tasknm)+4, " [%16s]", tasknm);
		else
			snprintf(tmp, sizeof(tasknm)+4, "  %16s ", tasknm);
		strncat(buf, tmp, TMPMAX);

		/* Is this the "main" thread of a multithreaded process?
		 * We check by seeing if (a) it's a userspace thread,
		 * (b) it's TGID == it's PID, and (c), there are >1 threads in
		 * the process.
		 * If so, display the number of threads in the overall process
		 * to the right..
		 */
		nr_thrds = get_nr_threads(g);
		if (g->mm && (g->tgid == t->pid) && (nr_thrds > 1)) {
			snprintf(tmp, TMPMAX-1, " %3d", nr_thrds);
			strncat(buf, tmp, TMPMAX);
		}

		snprintf(tmp, 2, "\n");
		strncat(buf, tmp, 2);
		pr_info("%s", buf);

		total++;
		memset(buf, 0, sizeof(buf));
		memset(tmp, 0, sizeof(tmp));
		task_unlock(t);
	} while_each_thread(g, t);
#if 0
	/* <same as above, reg the reader-writer spinlock for the task list> */
	read_unlock(&tasklist_lock);
#endif

	return total;
}

static int __init thrd_showall_fixed_init(void)
{
	int total;

	pr_info("%s: inserted\n", OURMODNAME);
	total = showthrds_fixed();
	pr_info("%s: total # of threads on the system: %d\n",
		OURMODNAME, total);

	return 0;		/* success */
}

static void __exit thrd_showall_fixed_exit(void)
{
	pr_info("%s: removed\n", OURMODNAME);
}

module_init(thrd_showall_fixed_init);
module_exit(thrd_showall_fixed_exit);
