/*
 * 6_percpuvar/6_percpuvar.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch 10 : Synchronization Primitives and How to Use Them
 ****************************************************************
 * Brief Description:
 *
 * For details, please refer the book, Ch 10.
 */
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

#define OURMODNAME   "6_percpuvar"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKDC book:ch/: hello, world");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

#define SHOW_CPU_CTX()  do {                                           \
	pr_info("%s():%d: *** thread PID %d on cpu %d now ***\n",      \
		__func__, __LINE__, current->pid, smp_processor_id()); \
} while(0)

#define MAX_KTHRDS       2
#define THRD0_ITERS      3
#define THRD1_ITERS      3

static long (*ptr_sched_setaffinity)(pid_t, const struct cpumask *);
static struct task_struct *arr_tsk[MAX_KTHRDS];

/*--- The percpu variables, an integer 'pcpa' and a dat structure --- */
/* This integer is statically allocated and initialized to 0 */
DEFINE_PER_CPU(int, pcpa);
/* This will be dynamically allocated via __alloc_percpu() */
struct drv_ctx {
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
	int i, val;
	long thrd = (long)arg;
	struct drv_ctx *ctx;

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

	if (thrd == 0) { /* our kthread #0 runs on CPU 0 */
		for (i=0; i<THRD0_ITERS; i++) {
			/* Operate on our perpcu integer */
			val = ++ get_cpu_var(pcpa);
			pr_info("cpu0: pcpa = %+d\n", val);
			put_cpu_var(pcpa);

			/* Operate on our perpcu structure */
			ctx = get_cpu_ptr(pcp_ctx);
			ctx->tx += 100;
			pr_info("cpu0: pcp ctx: tx = %5d, rx = %5d\n",
				ctx->tx, ctx->rx);
			put_cpu_ptr(pcp_ctx);
		}
	} else if (thrd == 1) { /* our kthread #1 runs on CPU 1 */
		for (i=0; i<THRD1_ITERS; i++) {
			/* Operate on our perpcu integer */
			val = -- get_cpu_var(pcpa);
			pr_info("cpu1: pcpa = %+d\n", val);
			put_cpu_var(pcpa);

			/* Operate on our perpcu structure */
			ctx = get_cpu_ptr(pcp_ctx);
			ctx->rx += 200;
			pr_info("cpu1: pcp ctx: tx = %5d, rx = %5d\n",
				ctx->tx, ctx->rx);
			put_cpu_ptr(pcp_ctx);
		}
	}

	disp_vars();
	pr_info("Our kernel thread #%ld exiting now...\n", thrd);
	do_exit(0);
}

/*
 * run_kthrd()
 * Creates and wakes up a kernel thread; a simple wrapper over the kernel's
 * kthread_run() wrapper.
 * Be sure to call our stop_kthrd() wrapper upon cleanup.
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

static int __init init_6_percpuvar(void)
{
	pr_debug("%s: inserted\n", OURMODNAME);

	/* WARNING! This is considered a hack.
	 * As sched_setaffinity() isn't exported, we don't have access to it
	 * within this kernel module. So, here we resort to a hack: we use
	 * kallsyms_lookup_name() (which works when CONFIG_KALLSYMS is defined)
	 * to retrieve the function pointer, subsequently calling the function
	 * via it's pointer (with 'C' what you do is only limited by your
	 * imagination :).
	 */
	ptr_sched_setaffinity = (void *)kallsyms_lookup_name("sched_setaffinity");
	if (!ptr_sched_setaffinity) {
		pr_warn("%s: couldn't obtain sched_setaffinity() addr via "
			"kallsyms_lookup_name(), aborting ...\n", OURMODNAME);
		return -ENOSYS;
	}

	/* Dynamically allocate the percpu structures */
	pcp_ctx = __alloc_percpu(sizeof(struct drv_ctx), sizeof(void *));
	if (!pcp_ctx) {
		pr_info("%s: kthread thrd #0 not created, aborting...\n",
			OURMODNAME);
		return -ENOMEM;
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
		return -ENOMEM;
	}

	return 0;		/* success */
}

static int stop_kthrd(struct task_struct *tsk)
{
	int ret = 0;

	if (tsk) 
		ret = kthread_stop(tsk);
	put_task_struct(tsk); // dec refcnt, "release" the task struct
	return ret;
}

static void __exit exit_6_percpuvar(void)
{
	stop_kthrd(arr_tsk[0]);
	stop_kthrd(arr_tsk[1]);
	disp_vars();
	free_percpu(pcp_ctx);
	pr_debug("%s: removed.\n", OURMODNAME);
}

module_init(init_6_percpuvar);
module_exit(exit_6_percpuvar);
