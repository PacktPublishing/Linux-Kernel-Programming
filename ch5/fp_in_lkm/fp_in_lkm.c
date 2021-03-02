/*
 * ch5/fp_in_lkm/fp_in_lkm.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming
 *
 * From: Ch 5: Writing your First Kernel Module - LKMs Part 2
 ****************************************************************
 * Brief Description:
 * A quick demo to show that we cannot / must not attempt to perform FP
 * (floating point) arithmetic in kernel mode.
 *
 * For details, please refer the book, Ch 5.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/fpu/api.h>

#define OURMODNAME   "fp_in_lkm"

MODULE_AUTHOR("<insert your name here>");
MODULE_DESCRIPTION("LKP book:ch5/fp_in_lkm: no performing FP \
(floating point) arithmetic in kernel mode");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

static double num = 22.0, den = 7.0, mypi;

static int __init fp_in_lkm_init(void)
{
	pr_info("%s: inserted\n", OURMODNAME);

	kernel_fpu_begin();
	mypi = num / den;
	kernel_fpu_end();
#if 1
	pr_info("%s: PI = %.4f = %.4f\n", OURMODNAME, mypi, num / den);
#endif

	return 0;		/* success */
}

static void __exit fp_in_lkm_exit(void)
{
	kernel_fpu_begin();
	pr_info("%s: mypi = %f\n", OURMODNAME, mypi);
	kernel_fpu_end();

	pr_info("%s: removed\n", OURMODNAME);
}

module_init(fp_in_lkm_init);
module_exit(fp_in_lkm_exit);
