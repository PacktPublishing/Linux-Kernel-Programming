/*
 * 2_rmw_atomic_bitops/rmw_atomic_bitops.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming
 *
 * From: Ch 13 : Kernel synchronization, Part 2
 ****************************************************************
 * Brief Description:
 * A quick demo showing the usage of the RMW (Read Modify Write) atomic bitwise
 * APIs. Here, there's no device, so we simply use these APIs on a RAM variable!
 *
 * For details, please refer the book, Ch 13.
 */
//#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>
#include <linux/bitops.h>
#include "../../convenient.h"

#define OURMODNAME   "2_rmw_atomic_bitops"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION(
"LKP book:ch13/2_rmw_atomic_bitops: quick demo of the RMW atomic bitwise operators");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

#define SHOW(n, p, msg) do {                                   \
	pr_info("%2d:%27s: mem : %3ld = 0x%02lx\n", n, msg, p, p); \
} while (0)

static unsigned long mem;
static u64 t1, t2;
static int MSB = BITS_PER_BYTE - 1;
DEFINE_SPINLOCK(slock);

/* Set the MSB; optimally, with the set_bit() RMW atomic API */
static inline void setmsb_optimal(int i)
{
	t1 = ktime_get_real_ns();
	set_bit(MSB, &mem);
	t2 = ktime_get_real_ns();
	SHOW(i, mem, "set_bit(7,&mem)");
	SHOW_DELTA(t2, t1);
}
/* Set the MSB; the traditional way, using a spinlock to protect the RMW
 * critical section
 */
static inline void setmsb_suboptimal(int i)
{
	u8 tmp;

	t1 = ktime_get_real_ns();
	spin_lock(&slock);
	/* critical section: RMW : read, modify, write */
	tmp = mem;
	tmp |= 0x80;   // 0x80 = 1000 0000 binary
	mem = tmp;
	spin_unlock(&slock);
	t2 = ktime_get_real_ns();

	SHOW(i, mem, "set msb suboptimal: 7,&mem");
	SHOW_DELTA(t2, t1);
}

static int __init atomic_rmw_bitops_init(void)
{
	int i = 1, ret;

	pr_info("%s: inserted\n", OURMODNAME);

	SHOW(i++, mem, "at init");

	setmsb_optimal(i++);
	setmsb_suboptimal(i++);

	clear_bit(MSB, &mem);
	SHOW(i++, mem, "clear_bit(7,&mem)");

	change_bit(MSB, &mem);
	SHOW(i++, mem, "change_bit(7,&mem)");

	ret = test_and_set_bit(0, &mem);
	SHOW(i++, mem, "test_and_set_bit(0,&mem)");
	pr_info("       ret = %d\n", ret);

	ret = test_and_clear_bit(0, &mem);
	SHOW(i++, mem, "test_and_clear_bit(0,&mem)");
	pr_info("       ret (prev value of bit 0) = %d\n", ret);

	ret = test_and_change_bit(1, &mem);
	SHOW(i++, mem, "test_and_change_bit(1,&mem)");
	pr_info("       ret (prev value of bit 1) = %d\n", ret);

	pr_info("%2d: test_bit(%d-0,&mem):\n", i, MSB);
	for (i = MSB; i >= 0; i--)
		pr_info("  bit %d (0x%02lx) : %s\n", i, BIT(i), test_bit(i, &mem)?"set":"cleared");

	return 0;		/* success */
}

static void __exit atomic_rmw_bitops_exit(void)
{
	mem = 0x0;
	pr_info("%s: removed\n", OURMODNAME);
}

module_init(atomic_rmw_bitops_init);
module_exit(atomic_rmw_bitops_exit);
