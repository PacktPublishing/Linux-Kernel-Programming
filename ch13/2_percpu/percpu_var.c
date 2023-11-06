/*
 * ch13/2_percpu/percpu_var.c
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
 * FYI: we use a very hack-y approach to accessing the unexported symbol
 * sched_setaffinity(); details follow. We get away with it here, but
 * DON'T use this approach in production.
 *
 * For details, please refer the book, Ch 13.
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/sched/task.h>  // {get,put}_task_struct()
#include <linux/percpu.h>
#include <linux/kthread.h>
#include <linux/cpumask.h>
#include <linux/kallsyms.h>
#include <linux/cred.h>
#include <linux/delay.h>
#include "../../convenient.h"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKP book:ch13/2_percpu: demo of using percpu variables");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");


#define SHOW_CPU_CTX() do {                        \
	pr_info("*** kthread PID %d on cpu %d now ***\n",\
		current->pid, smp_processor_id()); \
} while(0)

#define MAX_KTHRDS       2
#define THRD0_ITERS      3
#define THRD1_ITERS      3

static unsigned long func_ptr;
module_param(func_ptr, ulong, 0);

// schedsa_ptr is our function pointer to the sched_setaffinity() function
unsigned long (*schedsa_ptr)(pid_t, const struct cpumask *) = NULL;

static struct task_struct *arr_tsk[MAX_KTHRDS];

/*--- The percpu variables, an integer 'pcpa' and a data structure --- */
/* This percpu integer 'pcpa' is statically allocated and initialized to 0 */
DEFINE_PER_CPU(int, pcpa);

/* This percpu structure will be dynamically allocated via the alloc_percpu() */
static struct drv_ctx {
	int tx, rx; /* here, as a demo, we just use these two members,
	               ignoring the rest */
	int err, myword;
	u32 config1, config2;
	u64 config3;
} *pcp_ctx;

/* Display the percpu vars */
static inline void disp_vars(void)
{
	int i, val, rx, tx;

	PRINT_CTX();
	for_each_online_cpu(i) {
		val = per_cpu(pcpa, i);
		rx = per_cpu_ptr(pcp_ctx, i)->rx;
		tx = per_cpu_ptr(pcp_ctx, i)->tx;
		pr_info(" cpu %2d: pcpa = %+d, rx = %5d, tx = %5d\n",
			i, val, rx, tx);
	}
}

static long set_cpuaffinity(unsigned int cpu)
{
	struct cpumask mask;
	long ret = 0;

	unsigned int euid = from_kuid(&init_user_ns, current_euid());
	struct cred *new;

	/*
	 * Not root? get root! (hey, we're in kernel mode :)
	 * This isn't really required; we're just showing off...
	 */
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
		pr_info("after commit_creds(): uid=%u euid=%u\n",
			from_kuid(&init_user_ns, current_uid()),
			from_kuid(&init_user_ns, current_euid()));
	}

	/* pr_info("setting cpu mask to cpu #%u now...\n", cpu); */
	cpumask_clear(&mask);
	cpumask_set_cpu(cpu, &mask); // 1st param is the CPU number, not bitmask
	/* !HACK! sched_setaffinity() is NOT exported, we can't call it
         * sched_setaffinity(0, &mask);  // 0 => on self 
	 * so we invoke it via it's function pointer
	 */
	ret = (*schedsa_ptr)(0, &mask);  // 0 => on self

	return ret;
}

/* Our kernel thread worker routine */
static int thrd_work(void *arg)
{
	int i, val;
	long thrd = (long)arg;
	struct drv_ctx *ctx;

	if (thrd != 0 && thrd != 1) {
		pr_err("'thrd' value invalid (thrd=%ld)\n", thrd);
		return -EINVAL;
	}

	/* Set CPU affinity mask to 'thrd', which is either 0 or 1 */
	if (set_cpuaffinity(thrd) < 0) {
		pr_err("setting cpu affinity mask for our kthread %ld failed\n", thrd);
		return -ENOSYS;
	}
	SHOW_CPU_CTX();

	if (thrd == 0) { /* our kthread #0 runs on CPU 0 */
		for (i=0; i<THRD0_ITERS; i++) {
			/* Operate on our perpcu integer */
			val = ++ get_cpu_var(pcpa);
			pr_info("  thrd_0/cpu0: pcpa = %+d\n", val);
			put_cpu_var(pcpa);

			/* Operate on our perpcu structure */
			ctx = get_cpu_ptr(pcp_ctx);
			ctx->tx += 100;
			pr_info("  thrd_0/cpu0: pcp ctx: tx = %5d, rx = %5d\n",
				ctx->tx, ctx->rx);
			put_cpu_ptr(pcp_ctx);
		}
	} else if (thrd == 1) { /* our kthread #1 runs on CPU 1 */
		for (i=0; i<THRD1_ITERS; i++) {
			/* Operate on our perpcu integer */
			val = -- get_cpu_var(pcpa);
			pr_info("  thrd_1/cpu1: pcpa = %+d\n", val);
			put_cpu_var(pcpa);

			/* Operate on our perpcu structure */
			ctx = get_cpu_ptr(pcp_ctx);
			ctx->rx += 200;
			pr_info("  thrd_1/cpu1: pcp ctx: tx = %5d, rx = %5d\n",
				ctx->tx, ctx->rx);
			put_cpu_ptr(pcp_ctx);
		}
	}

	disp_vars();
	pr_info("Our kernel thread #%ld exiting now...\n", thrd);
	return 0;
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
		pr_err(" kthread_run() for our kthread %ld failed\n", thrdnum);
		return -1;
	}
	get_task_struct(arr_tsk[thrdnum]); /* inc refcnt, "take" the task
		* struct, ensuring that the task does not simply die */

	return 0;
}


static int __init init_percpu_var(void)
{
	int ret = 0;

	pr_info("inserted\n");

	/* !WARNING! This is considered a hack.
	 * As sched_setaffinity() isn't exported, we don't have access to it
	 * within this kernel module. So, here we resort to a hack: 
	 * a) Until 5.7, we could directly use the kallsyms_lookup_name() function
	 *    (which works when CONFIG_KALLSYMS is defined) to retrieve the function
	 *    pointer, and subsequently call the function via it's pointer (with 'C'
	 *    what you do is only limited by your imagination :).
	 * b) From 5.7 on, the kernel devs unexported the kallsyms_lookup_name()!
	 *    (Commit id 0bd476e6c671. Rationale: https://lwn.net/Articles/813350/).
	 *    With it gone, we now simply use this approach: a helper script greps
	 *    the kallsyms_lookup_name() address and passes it to this module! We
	 *    equate it to the exepcted function signature - that of
	 *    sched_setaffinity() - and use it.
	 * *Not* pedantically right, but hey, it works. Don't do this in production.
	 */
	ret = -ENOSYS;
	if (!func_ptr) {
		pr_warn("%s: couldn't obtain sched_setaffinity() addr via "
			"module param, aborting ...\n", KBUILD_MODNAME);
		return ret;
	}
	// set up the function pointer
	schedsa_ptr = (unsigned long (*)(pid_t pid, const struct cpumask *in_mask))func_ptr;

	/* Dynamically allocate the percpu structures */
	ret = -ENOMEM;
	pcp_ctx = (struct drv_ctx __percpu *) alloc_percpu(struct drv_ctx);
	if (!pcp_ctx) {
		pr_info("alloc_percpu() failed, aborting...\n");
		goto out1;
	}

	/* Spawn two kernel threads */
	ret = -ENOMEM;
	if (run_kthrd("thrd_0", 0) < 0) {
		pr_info("kthread thrd #0 not created, aborting...\n");
		goto out2;
	}
	if (run_kthrd("thrd_1", 1) < 0) {
		pr_info("kthread thrd #1 not created, aborting...\n");
		goto out3;
	}

	return 0;		/* success */

out3:
	kthread_stop(arr_tsk[0]);
out2:
	free_percpu(pcp_ctx);
out1:
	return ret;
}

static void __exit exit_percpu_var(void)
{
	kthread_stop(arr_tsk[0]);
	kthread_stop(arr_tsk[1]);
	disp_vars();
	free_percpu(pcp_ctx);
	pr_info("removed.\n");
}

module_init(init_percpu_var);
module_exit(exit_percpu_var);
