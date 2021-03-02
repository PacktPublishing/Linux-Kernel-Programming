/*
 * ch6/current_affairs/current_affairs.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming
 *
 * From: Ch 6: Kernel and Memory Management Internals -Essentials
 ****************************************************************
 * Brief Description:
 *
 * For details, please refer the book, Ch 6.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>	/* current() */
#include <linux/preempt.h>	/* in_task() */
#include <linux/cred.h>		/* current_{e}{u,g}id() */
#include <linux/uidgid.h>	/* {from,make}_kuid() */

#define OURMODNAME   "current_affairs"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKP book:ch6/current_affairs: display a few members of"
" the current process' task structure");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static inline void show_ctx(char *nm)
{
	/* Extract the task UID and EUID using helper methods provided */
	unsigned int uid = from_kuid(&init_user_ns, current_uid());
	unsigned int euid = from_kuid(&init_user_ns, current_euid());

	pr_info("%s:%s():%d ", nm, __func__, __LINE__);
	if (likely(in_task())) {
		pr_info("%s: in process context ::\n"
			" PID         : %6d\n"
			" TGID        : %6d\n"
			" UID         : %6u\n"
			" EUID        : %6u (%s root)\n"
			" name        : %s\n"
			" current (ptr to our process context's task_struct) :\n"
			"               0x%pK (0x%px)\n"
			" stack start : 0x%pK (0x%px)\n", nm,
			/* always better to use the helper methods provided */
			task_pid_nr(current), task_tgid_nr(current),
			/* ... rather than the 'usual' direct lookups:
			 * current->pid, current->tgid,
			 */
			uid, euid,
			(euid == 0 ? "have" : "don't have"),
			current->comm,
			current, current,
			current->stack, current->stack);
	} else
		pr_alert("%s: in interrupt context [Should NOT Happen here!]\n", nm);
}

static int __init current_affairs_init(void)
{
	pr_info("%s: inserted\n", OURMODNAME);
	pr_info(" sizeof(struct task_struct)=%zd\n", sizeof(struct task_struct));
	show_ctx(OURMODNAME);
	return 0;		/* success */
}

static void __exit current_affairs_exit(void)
{
	show_ctx(OURMODNAME);
	pr_info("%s: removed\n", OURMODNAME);
}

module_init(current_affairs_init);
module_exit(current_affairs_exit);
