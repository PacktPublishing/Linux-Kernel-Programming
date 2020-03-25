/*
 * ch6/current_affairs/current_affairs.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 6: Kernel and Memory Management Internals -Essentials
 ****************************************************************
 * Brief Description:
 *
 * For details, please refer the book, Ch 6.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>   /* current() */
#include <linux/preempt.h> /* in_interrupt() */
#include <linux/cred.h>    /* current_{e}{u,g}id() */
#include <linux/uidgid.h>  /* {from,make}_kuid() */

#define OURMODNAME   "current_affairs"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LLKD book:ch4/current_affairs: display a few members of"
		" the current process' task structure");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static inline void show_ctx(char *nm)
{
	unsigned int uid = from_kuid(&init_user_ns, current_uid());
	unsigned int euid = from_kuid(&init_user_ns, current_euid());

	pr_info("%s:%s():%d ", nm, __func__, __LINE__);
	if (likely(!in_interrupt())) {
		pr_info(
		"%s: in process context ::\n"
		" PID         : %6d\n"
		" TGID        : %6d\n"
		" UID         : %6u\n"
		" EUID        : %6u (%s root)\n"
		" name        : %s\n"
		" current (ptr to our process context's task_struct) :\n"
		"               0x%pK (0x%16lx)\n"
		" stack start : 0x%pK (0x%016lx)\n",
		nm, current->pid, current->tgid, uid, euid,
		(euid == 0?"have":"don't have"), current->comm,
		current, (long unsigned)current,
		current->stack, (long unsigned)current->stack);
	} else
		pr_alert("%s: in interrupt context [Should NOT Happen here!]\n",
			nm);
}

static int __init current_affairs_init(void)
{
	pr_debug("%s: inserted\n", OURMODNAME);
	show_ctx(OURMODNAME);
	return 0;		/* success */
}

static void __exit current_affairs_exit(void)
{
	show_ctx(OURMODNAME);
	pr_debug("%s: removed\n", OURMODNAME);
}

module_init(current_affairs_init);
module_exit(current_affairs_exit);
