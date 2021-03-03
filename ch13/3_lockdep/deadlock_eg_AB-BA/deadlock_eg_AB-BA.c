/*
 * ch13/3_lockdep/deadlock_eg_AB-BA/deadlock_eg_AB-BA.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming
 *
 * From: Ch 13: Kernel Synchronization, Part 2 
 ****************************************************************
 * Brief Description:
 * Here we deliberately violate our lock ordering rule, thus ending up with a
 * classic AB-BA deadlock. Running a debug kernel, we expect lockdep to catch
 * and report it!
 *
 * For details, please refer the book, Ch 13.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/sched/task.h>  // {get,put}_task_struct()
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/cpumask.h>
#include <linux/kallsyms.h>
#include <linux/cred.h>
#include "../../../convenient.h"

#define OURMODNAME   "deadlock_eg_AB-BA"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKP book:ch13/3_lockdep/deadlock_eg_AB-BA: small demo of "
"deliberately setting up an AB-BA deadlock; lockdep catches it");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/* module param: set to 1 to perform out-of-order (ooo) locking */
static int lock_ooo;
module_param(lock_ooo, int, 0644);
MODULE_PARM_DESC(lock_ooo, "set to 1 to perform out-of-order (ooo) locking (defaults to 0)");

#define SHOW_CPU_CTX()  do {                                   \
	pr_info("%s():%d: *** thread PID %d on cpu %d now ***\n",  \
		__func__, __LINE__, current->pid, smp_processor_id()); \
} while(0)

#define MAX_KTHRDS       2
#define THRD0_ITERS      1
#define THRD1_ITERS      1

/* Locking rule : lockA --> lockB */
DEFINE_SPINLOCK(lockA);
DEFINE_SPINLOCK(lockB);
  /* Below, when lock_ooo is 1, we deliberately violate this locking rule ! */

static long (*ptr_sched_setaffinity)(pid_t, const struct cpumask *);
static struct task_struct *arr_tsk[MAX_KTHRDS];

static long set_cpuaffinity(unsigned int cpu)
{
	struct cpumask mask;
	long ret = 0;
	unsigned int euid = from_kuid(&init_user_ns, current_euid());
	struct cred *new;

	/* Not root? get root! (hey, we're in kernel mode :) */
	if (unlikely(euid)) {
		pr_info("%s(): before commit_creds(): uid=%u euid=%u\n",
			__func__, from_kuid(&init_user_ns, current_uid()),
			from_kuid(&init_user_ns, current_euid()));

		new = prepare_creds();
		if (!new)
			return -ENOMEM;

		new->fsuid = new->uid = new->euid = make_kuid(current_user_ns(), 0);
		if (!uid_valid(new->uid))
			return -1;
		commit_creds(new);
		pr_info("%s(): after commit_creds(): uid=%u euid=%u\n",
			__func__, from_kuid(&init_user_ns, current_uid()),
			from_kuid(&init_user_ns, current_euid()));
	}

	//pr_info("%s(): setting cpu mask to cpu #%u now...\n", __func__, cpu);
	cpumask_clear(&mask);
	cpumask_set_cpu(cpu, &mask); // 1st param is the CPU number, not bitmask
	/* !HACK! sched_setaffinity() is NOT exported, we can't call it
         * sched_setaffinity(0, &mask);  // 0 => on self 
	 * so we invoke it via func pointer */
	ret = (*ptr_sched_setaffinity)(0, &mask);  // 0 => on self

	return ret;
}

/* Our kernel thread worker routine */
static int thrd_work(void *arg)
{
	int i=0;
	long thrd = (long)arg;

	if (thrd != 0 && thrd != 1) {
		pr_err("%s: 'thrd' value invalid (thrd=%ld)\n", OURMODNAME, thrd);
		return -EINVAL;
	}

	/* Set CPU affinity mask to 'thrd', which is either 0 or 1 */
	if (set_cpuaffinity(thrd) < 0) {
		pr_err("%s: setting cpu affinity mask for our kthread"
			" %ld failed\n", OURMODNAME, thrd);
		return -ENOSYS;
	}
	SHOW_CPU_CTX();

	/* Locking rule : lockA --> lockB */

	if (thrd == 0) { /* our kthread #0 runs on CPU 0 */
		pr_info(" Thread #%ld: locking: we do:"
			" lockA --> lockB\n", thrd);

		for (i = 0; i < THRD0_ITERS; i ++) {
			/* In this thread, perform the locking per the lock ordering 'rule';
	         * first take lockA, then lockB */
			pr_info(" iteration #%d on cpu #%ld\n", i, thrd);

			spin_lock(&lockA);
			DELAY_LOOP('A', 3);
			spin_lock(&lockB);
			DELAY_LOOP('B', 2);
			spin_unlock(&lockB);
			spin_unlock(&lockA);
		}
	} else if (thrd == 1) { /* our kthread #1 runs on CPU 1 */

		for (i = 0; i < THRD1_ITERS; i ++) {
			/* In this thread, if the parameter lock_ooo is 1, *violate* the
			 * lock ordering 'rule'; first (attempt to) take lockB, then lockA */
			pr_info(" iteration #%d on cpu #%ld\n", i, thrd);

			if (lock_ooo == 1) {		// violate the rule, naughty !
				pr_info(" Thread #%ld: locking: we do: lockB --> lockA\n",thrd);
				spin_lock(&lockB);
				DELAY_LOOP('B', 2);
				spin_lock(&lockA);
				DELAY_LOOP('A', 3);
				spin_unlock(&lockA);
				spin_unlock(&lockB);
			} else if (lock_ooo == 0) {		// follow the rule, good !
				pr_info(" Thread #%ld: locking: we do: lockA --> lockB\n",thrd);
				spin_lock(&lockA);
				DELAY_LOOP('B', 2);
				spin_lock(&lockB);
				DELAY_LOOP('A', 3);
				spin_unlock(&lockB);
				spin_unlock(&lockA);
			}
		}
	}
	pr_info("Our kernel thread #%ld exiting now...\n", thrd);
	do_exit(0);
}

/*
 * run_kthrd()
 * Creates and wakes up a kernel thread; a simple wrapper over the kernel's
 * kthread_run() wrapper.
 * Be sure to call the kthread_stop() routine upon cleanup.
 */
static int run_kthrd(char *kname, long thrdnum)
{
	/* 2nd arg is (void * arg) to pass, ret val is task ptr on success */
	arr_tsk[thrdnum] = kthread_run(thrd_work, (void *)thrdnum,
					"%s/%ld", kname, thrdnum);
	if (!arr_tsk[thrdnum]) {
		pr_err(" kthread_create() for our kthread %ld failed\n", thrdnum);
		return -1;
	}
	get_task_struct(arr_tsk[thrdnum]); /* inc refcnt, "take" the task
		struct, ensuring that the task does not simply die */

	return 0;
}

static int __init deadlock_eg_AB_BA_init(void)
{
	pr_info("%s: inserted (param: lock_ooo=%d)\n", OURMODNAME, lock_ooo);

	/* WARNING! This is considered a hack.
	 * As sched_setaffinity() isn't exported, we don't have access to it
	 * within this kernel module. So, here we resort to a hack: we use
	 * kallsyms_lookup_name() (which works when CONFIG_KALLSYMS is defined)
	 * to retrieve the function pointer, subsequently calling the function
	 * via it's pointer (with 'C' what you do is only limited by your
	 * imagination :).
	 *
	 * EDIT + *NOTE*
	 * The kallsyms_lookup_name() API has been unexported from kernel ver 5.7
	 * Commit: https://github.com/torvalds/linux/commit/0bd476e6c67190b5eb7b6e105c8db8ff61103281
	 * Rationale: https://lwn.net/Articles/813350/
	 */
	ptr_sched_setaffinity = (void *)kallsyms_lookup_name("sched_setaffinity");
	if (!ptr_sched_setaffinity) {
		pr_warn("%s: couldn't obtain sched_setaffinity() addr via "
			"kallsyms_lookup_name(), aborting ...\n", OURMODNAME);
		return -ENOSYS;
	}

	/* Spawn two kernel threads */
	if (run_kthrd("thrd_0", 0) < 0) {
		pr_info("%s: kthread thrd #0 not created, aborting...\n",
			OURMODNAME);
		return -ENOMEM;
	}
	if (run_kthrd("thrd_1", 1) < 0) {
		pr_info("%s: kthread thrd #1 not created, aborting...\n",
			OURMODNAME);
		kthread_stop(arr_tsk[0]);
		return -ENOMEM;
	}

	return 0;		/* success */
}

static void __exit deadlock_eg_AB_BA_exit(void)
{
	kthread_stop(arr_tsk[0]);
	kthread_stop(arr_tsk[1]);
	pr_info("%s: removed.\n", OURMODNAME);
}

module_init(deadlock_eg_AB_BA_init);
module_exit(deadlock_eg_AB_BA_exit);
