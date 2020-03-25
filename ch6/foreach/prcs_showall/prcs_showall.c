/*
 * ch6/foreach/prcs_showall.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 6 : Kernel and MM Internals - Essentials
 ****************************************************************
 * Brief Description:
 * This kernel module iterates over the task structures of all *processes*
 * currently alive on the box, printing out a few details for each of them.
 * We use the for_each_process() macro to do so here.
 *
 * For details, please refer the book, Ch 6.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>

#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 10, 0)
#include <linux/sched/signal.h>	/* for_each_xxx(), ... */
#endif
#include <linux/fs.h>		/* no_llseek() */
#include <linux/slab.h>
#include <linux/uaccess.h>	/* copy_to_user() */
#include <linux/kallsyms.h>

#define OURMODNAME	"prcs_showall"

MODULE_AUTHOR("<insert your name here>");
MODULE_DESCRIPTION("LLKD book:ch4/foreach/prcs_showall: "
		   "Show all processes by iterating over the task list");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static int show_prcs_in_tasklist(void)
{
	struct task_struct *p;
#define MAXLEN   128
	char tmp[MAXLEN];
	int numread = 0, n = 0, total = 0;
	char hdr[] = "     Name       | TGID  |  PID  | RUID  | EUID";

	pr_info("%s\n", &hdr[0]);
	for_each_process(p) {
		memset(tmp, 0, 128);
		n = snprintf(tmp, 128, "%-16s|%7d|%7d|%7u|%7u\n",
			p->comm, p->tgid, p->pid,
			/* (old way): p->uid, p->euid
		        current_uid().val, current_euid().val */
		     __kuid_val(p->cred->uid), __kuid_val(p->cred->euid)
		    );
		numread += n;
		pr_info("%s", tmp);
		//pr_debug("n=%d numread=%d tmp=%s\n", n, numread, tmp);

		cond_resched();
		total++;
	}			// for_each_process()

	return total;
}

static int __init prcs_showall_init(void)
{
	int total;

	pr_debug("%s: inserted\n", OURMODNAME);
	total = show_prcs_in_tasklist();
	pr_info("%s: total # of processes on system: %d\n", OURMODNAME, total);

	return 0;		/* success */
}

static void __exit prcs_showall_exit(void)
{
	pr_debug("%s: removed\n", OURMODNAME);
}

module_init(prcs_showall_init);
module_exit(prcs_showall_exit);
