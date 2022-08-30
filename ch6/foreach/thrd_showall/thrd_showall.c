/*
 * ch6/foreach/thrd_showall/thrd_showall.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming
 *
 * From: Ch 6 : Kernel and MM Internals Essentials
 ****************************************************************
 * Brief Description:
 * This kernel module iterates over the task structures of all *threads*
 * currently alive on the box, printing out some details.
 * We use the do_each_thread() { ... } while_each_thread() macros to do
 * so here.
 *
 * For details, please refer the book, Ch 6.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>     /* current() */
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 10, 0)
#include <linux/sched/signal.h>
#endif

#define OURMODNAME   "thrd_showall"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKP book:ch6/foreach/thrd_showall:"
" demo to display all threads by iterating over the task list");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/* Display just CPU 0's idle thread, i.e., the pid 0 task,
 * the (terribly named) 'swapper/n'; n = 0, 1, 2,...
 * Again, init_task is always the task structure of the first CPU's
 * idle thread, i.e., we're referencing swapper/0.
 */
static inline void disp_idle_thread(void)
{
	struct task_struct *t = &init_task;

	/* We know that the swapper is a kernel thread */
	pr_info("%8d %8d   0x%px  0x%px [%16s]\n",
		t->pid, t->pid, t, t->stack, t->comm);
}

static int showthrds(void)
{
	struct task_struct *g = NULL, *t = NULL; /* 'g' : process ptr; 't': thread ptr */
	int nr_thrds = 1, total = 1;    /* total init to 1 for the idle thread */
#define BUFMAX		256
#define TMPMAX		128
	char buf[BUFMAX], tmp[TMPMAX];
	const char hdr[] =
"------------------------------------------------------------------------------------------\n"
"    TGID     PID         current           stack-start         Thread Name     MT? # thrds\n"
"------------------------------------------------------------------------------------------\n";

	pr_info("%s", hdr);
	disp_idle_thread();

	/*
	 * The do_each_thread() / while_each_thread() is a pair of macros that iterates over
	 * _all_ task structures in memory.
	 * The task structs are global of course; this implies we should hold a lock of some
	 * sort while working on them (even if only reading!). So, doing
	 *  read_lock(&tasklist_lock);
	 *  [...]
	 *  read_unlock(&tasklist_lock);
	 * BUT, this lock - tasklist_lock - isn't exported and thus unavailable to modules.
	 * So, using an RCU read lock is indicated here (this has been added later to this code).
	 * FYI: a) Ch 12 and Ch 13 cover the details on kernel synchronization.
	 *      b) Read Copy Update (RCU) is a complex synchronization mechanism; it's
	 * conceptually explained really well within this blog article:
	 *  https://reberhardt.com/blog/2020/11/18/my-first-kernel-module.html
	 */
	rcu_read_lock();
	do_each_thread(g, t) {     /* 'g' : process ptr; 't': thread ptr */
		task_lock(t);

		snprintf(buf, BUFMAX-1, "%8d %8d ", g->tgid, t->pid);

		/* task_struct addr and kernel-mode stack addr */
		snprintf(tmp, TMPMAX-1, "  0x%px", t);
		/*
		 * To concatenate the temp string to our buffer, we could go with the
		 * strncat() here; flawfinder, though, points out this is potentially
		 * dangerous; so we simply use another snprintf() to achieve the same.
		 * Why not use strlcat() instead? Here, it runs into trouble - being
		 * called in an atomic context, which isn't ok (due to the
		 * might_sleep() within it's code)...
		 */
		snprintf(buf, BUFMAX-1, "%s%s  0x%px", buf, tmp, t->stack);

		if (!g->mm) {	// kernel thread
		/* One might question why we don't use the get_task_comm() to obtain
		 * the task's name here; the short reason: it causes a deadlock! We
		 * shall explore this (and how to avoid it) in some detail in Ch 17 -
		 * Kernel Synchronization Part 2. For now, we just do it the simple way
		 */
			snprintf(tmp, TMPMAX-1, " [%16s]", t->comm);
		} else {
			snprintf(tmp, TMPMAX-1, "  %16s ", t->comm);
		}
		snprintf(buf, BUFMAX-1, "%s%s", buf, tmp);

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
			snprintf(buf, BUFMAX-1, "%s%s", buf, tmp);
		}

		snprintf(buf, BUFMAX-1, "%s\n", buf);
		pr_info("%s", buf);

		total++;
		memset(buf, 0, sizeof(buf));
		memset(tmp, 0, sizeof(tmp));
		task_unlock(t);
	} while_each_thread(g, t);
	rcu_read_unlock();

	return total;
}

static int __init thrd_showall_init(void)
{
	int total;

	pr_info("%s: inserted\n", OURMODNAME);
	total = showthrds();
	pr_info("%s: total # of threads on the system: %d\n",
		OURMODNAME, total);

	return 0;		/* success */
}

static void __exit thrd_showall_exit(void)
{
	pr_info("%s: removed\n", OURMODNAME);
}

module_init(thrd_showall_init);
module_exit(thrd_showall_exit);
