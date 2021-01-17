/*
 * solutions_to_assgn/ch12/dbgfs_showall_threads/dbgfs_showall_threads.c
 **************************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 12 : User-Kernel communication pathways
 **************************************************************************
 * Solution to Assignment, ch12, debugfs assigment #2:
 *
 * Write a kernel module that sets up a debugfs file here:
 *  <debugfs_mount_point>/dbgfs_showall_threads/dbgfs_showall_threads
 * when read, it should display some attributes of every thread alive. (Similar
 * to our code here: ch6/foreach/thrd_showall; there, though, the threads are
 * displayed only at insmod time; with a debugfs file, you can display info on
 * all threads at any time you choose to!
 * Suggested output is CSV format:
 *  TGID,PID,current,stack-start,name,#threads
 *   ([name] in square brackets => kernel thread;
 *   #threads only displays a positive integer; no output implies a
 *    single-threaded process; f.e.:
 *  130,130,0xffff9f8b3cd38000,0xffffc13280420000,[watchdogd])
 *
 * For details, please refer the book, Ch 12.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/debugfs.h>
#include <linux/vmalloc.h>

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#else
#include <asm/uaccess.h>
#endif

#define	OURMODNAME	"dbgfs_showall_threads"
MODULE_AUTHOR("<insert name here>");
MODULE_DESCRIPTION(
 "LLKD book:solutions_to_assgn/ch12/dbgfs_showall_threads: a soln to ch12 debugfs assignment #1");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

#define DBGFS_CREATE_ERR(pDentry, str) do {                     \
	pr_info("%s: failed.\n", str);                          \
	if (PTR_ERR(pDentry) == -ENODEV)                        \
		pr_info("%s: debugfs support not available?\n", \
		OURMODNAME);                                    \
	debugfs_remove_recursive(pDentry);	                \
	return PTR_ERR(pDentry);                                \
} while (0)

static struct dentry *gparent;

static ssize_t showall_threads(struct file *filp, char __user *ubuf,
			       size_t count, loff_t *fpos)
{
	struct task_struct *g, *t;	/* 'g' : process ptr; 't': thread ptr */
	char *data;
#define MEM_REQD_PER_THREAD 128
	int nr_thrds = 1, total = 1;   /* total init to 1 for the idle thread */
#define TMPMAX		128
	char tmp[TMPMAX];
	ssize_t numthrds = 0, n = 0;

	do_each_thread(g, t) {
		numthrds++;
	} while_each_thread(g, t);
	pr_debug("# threads alive: %zd\n", numthrds);

	/* We attempt to calculate the max amt of memory required; this,
	 * of course, is fraught with assumptions! It's a bit fragile;
	 * for this demo, we just leave it at this...
	 */
	data = vzalloc(numthrds * MEM_REQD_PER_THREAD);
	if (!data) {
		pr_alert("%s: vzalloc() failure!\n", OURMODNAME);
		return -ENOMEM;
	}

	/* The majority of the code below is pretty much a copy of this:
	 *  ch6/foreach/thrd_showall/thrd_showall.c:showthrds()
	 * And why not! Here, we change the *output format* to CSV, as the
	 * spec requires... simply loop around collecting all the output in
	 * our 'data' buffer and hand it off to userspace with just one call
	 * to simple_read_from_buffer().
	 */
#if 0
	/* the tasklist_lock reader-writer spinlock for the task list 'should'
	 * be used here, but, it's not exported, hence unavailable to our
	 * kernel module */
	read_lock(&tasklist_lock);
#endif
	/*--- All output in CSV format ---*/
	/* We know that the swapper is a kernel thread */
	snprintf(data, TMPMAX, "%d,%d,0x%px,0x%px,[%s]\n",
		 t->pid, t->pid, t, t->stack, t->comm);

	do_each_thread(g, t) {	/* 'g' : process ptr; 't': thread ptr */
		task_lock(t);

		memset(tmp, 0, sizeof(tmp));
		snprintf(tmp, TMPMAX - 1, "%d,%d", g->tgid, t->pid);
		strncat(data, tmp, TMPMAX);

		/* task_struct addr and kernel-mode stack addr */
		snprintf(tmp, TMPMAX - 1, ",0x%px", t);
		strncat(data, tmp, TMPMAX);
		snprintf(tmp, TMPMAX - 1, ",0x%px", t->stack);
		strncat(data, tmp, TMPMAX);

		if (!g->mm)	// kernel thread
		  /* One might question why we don't use the get_task_comm() to
		   * obtain the task's name here; the short reason: it causes a
		   * deadlock! We shall explore this (and how to avoid it) in
		   * some detail in the chapter on Synchronization. For now, we
		   * just do it the simple way ...
		   */
			snprintf(tmp, TMPMAX - 1, ",[%s]", t->comm);
		else
			snprintf(tmp, TMPMAX - 1, ",%s", t->comm);
		strncat(data, tmp, TMPMAX);

		/* Is this the "main" thread of a multithreaded process?
		 * We check by seeing if (a) it's a userspace thread,
		 * (b) it's TGID == it's PID, and (c), there are >1 threads in
		 * the process.
		 * If so, display the number of threads in the overall process
		 * to the right..
		 */
		nr_thrds = get_nr_threads(g);
		if (g->mm && (g->tgid == t->pid) && (nr_thrds > 1)) {
			snprintf(tmp, TMPMAX - 1, ",%d", nr_thrds);
			strncat(data, tmp, TMPMAX);
		}

		snprintf(tmp, 2, "\n");
		strncat(data, tmp, 2);
		total++;
		task_unlock(t);
	} while_each_thread(g, t);
#if 0
	/* <same as above, reg the reader-writer spinlock for the task list> */
	read_unlock(&tasklist_lock);
#endif
	n = simple_read_from_buffer(ubuf, strlen(data), fpos, data,
				    strlen(data));
	vfree(data);
	return n;
}

static const struct file_operations dbg_fops1 = {
	.read = showall_threads,
};

static int __init dbgfs_showall_threads_init(void)
{
	gparent = debugfs_create_dir(OURMODNAME, NULL);
	if (!gparent) {
		DBGFS_CREATE_ERR(gparent, "debugfs_create_dir");
	}

	if (!debugfs_create_file(OURMODNAME, 0444, gparent,
				 (void *)0, &dbg_fops1)) {
		DBGFS_CREATE_ERR(gparent, "debugfs_create_file 1");
	}
	pr_info
	    ("Debugfs entry setup successfully (under <debugfs_mount>/%s/%s)\n",
	     OURMODNAME, OURMODNAME);
	pr_info("%s initialized\n", OURMODNAME);
	return 0;		/* success */
}

static void __exit dbgfs_showall_threads_cleanup(void)
{
	debugfs_remove_recursive(gparent);
	pr_info("%s removed\n", OURMODNAME);
}

module_init(dbgfs_showall_threads_init);
module_exit(dbgfs_showall_threads_cleanup);
