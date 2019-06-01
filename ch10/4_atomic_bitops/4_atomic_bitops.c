/*
 * 4_atomic_bitops/4_atomic_bitops.c
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
 * For details, please refer the book, Ch .
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/bitops.h>

#define OURMODNAME   "4_atomic_bitops"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LKDC book:ch10/4_atomic_bitops: quick demo of the atomic bitwise operators");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

#define SHOW(n,p,msg) do {                                         \
	pr_info("%2d:%27s: mem : %3ld = 0x%02lx\n", n, msg, p, p); \
} while(0)

static unsigned long mem;
DEFINE_SPINLOCK(splock);

/* set the MSB */
static inline void setmsb_optimal(int i)
{
	set_bit(7, &mem);
	SHOW(i, mem, "set_bit(7,&mem)");
}
/* set the MSB */
static inline void setmsb_suboptimal(int i)
{
	u8 tmp;

	spin_lock(&splock);
	/* critical section: RMW : read, modify, write */
	tmp = mem;
	tmp |= 0x80;   // 0x80 = 1000 0000 binary
	mem = tmp;
	spin_unlock(&splock);

	SHOW(i, mem, "set msb suboptimal: 7,&mem");
}

static int __init atomic_bitops_init(void)
{
	int i = 1, ret;

	pr_debug("%s: inserted\n", OURMODNAME);

	SHOW(i++, mem, "at init");

	setmsb_optimal(i++);
	setmsb_suboptimal(i++);

	clear_bit(7, &mem);
	SHOW(i++, mem, "clear_bit(7,&mem)");

	change_bit(7, &mem);
	SHOW(i++, mem, "change_bit(7,&mem)");

	ret = test_and_set_bit(0, &mem);
	SHOW(i++, mem, "test_and_set_bit(0,&mem)");
	pr_info("       ret = %d\n", ret);

	ret = test_and_clear_bit(0, &mem);
	SHOW(i++, mem, "test_and_clear_bit(0,&mem)");
	pr_info("       ret = %d\n", ret);

	ret = test_and_change_bit(1, &mem);
	SHOW(i++, mem, "test_and_change_bit(1,&mem)");
	pr_info("       ret = %d\n", ret);

	pr_info("%2d: test_bit(7-0,&mem):\n", i);
	for (i=7; i>=0; i--)
		pr_info("  bit %d: %s\n", i, test_bit(i, &mem)?"set":"cleared");

	return 0;		/* success */
}

static void __exit atomic_bitops_exit(void)
{
	mem = 0x0;
	pr_debug("%s: removed\n", OURMODNAME);
}

module_init(atomic_bitops_init);
module_exit(atomic_bitops_exit);
