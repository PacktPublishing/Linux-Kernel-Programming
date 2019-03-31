/*
 * ch3/modstacking/core_lkm.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch 3: Writing your First Kernel Module- LKMs Part 2
 ****************************************************************
 * Brief Description:
 * This kernel module - core_lkm - is part of the 'modstacking' POC project:
 *    user_lkm
 *        |
 *    core_lkm           [<--- this code]
 * The user_lkm kernel module calls an (exported) function that resides 
 * in the core_lkm kernel module.
 *
 * For details, please refer the book, Ch 3.
 */
#include <linux/init.h>
#include <linux/module.h>

#define MODNAME   "core_lkm"
#define THE_ONE   0xfedface
MODULE_LICENSE("Dual MIT/GPL");

static int exp_int = 200;
EXPORT_SYMBOL_GPL(exp_int);

/* Functions to be called from other LKMs */

/* lkdc_sysinfo2:
 * a more security-aware version of the lkdc_sysinfo routine. We use
 * David Wheeler's flawfinder(1) tool to detect possible vulnerabilities;
 * so, we change the strlen, and replace the strncat with strlcat.
 */
static void lkdc_sysinfo2(void)
{
#define MSGLEN   128
	char msg[MSGLEN];

	memset(msg, 0, MSGLEN);
	snprintf(msg, 48, "%s(): minimal Platform Info:\nCPU: ", __func__);

	/* Strictly speaking, all this #if... is considered ugly and should be
	   isolated as far as is possible */
#ifdef CONFIG_X86
#if(BITS_PER_LONG == 32)
	strlcat(msg, "x86-32, ", MSGLEN);
#else
	strlcat(msg, "x86_64, ", MSGLEN);
#endif
#endif
#ifdef CONFIG_ARM
	strlcat(msg, "ARM-32, ", MSGLEN);
#endif
#ifdef CONFIG_ARM64
	strlcat(msg, "Aarch64, ", MSGLEN);
#endif
#ifdef CONFIG_MIPS
	strlcat(msg, "MIPS, ", MSGLEN);
#endif
#ifdef CONFIG_PPC
	strlcat(msg, "PowerPC, ", MSGLEN);
#endif
#ifdef CONFIG_S390
	strlcat(msg, "IBM S390, ", MSGLEN);
#endif

#ifdef __BIG_ENDIAN
	strlcat(msg, "big-endian; ", MSGLEN);
#else
	strlcat(msg, "little-endian; ", MSGLEN);
#endif

#if(BITS_PER_LONG == 32)
	strlcat(msg, "32-bit OS.\n", MSGLEN);
#elif(BITS_PER_LONG == 64)
	strlcat(msg, "64-bit OS.\n", MSGLEN);
#endif
	pr_info("%s", msg);
}
EXPORT_SYMBOL(lkdc_sysinfo2);

#if(BITS_PER_LONG == 32)
static u32 get_skey(int p)
#else   // 64-bit
static u64 get_skey(int p)
#endif
{
#if(BITS_PER_LONG == 32)
	u32 secret = 0x567def;
#else   // 64-bit
	u64 secret = 0x123abc567def;
#endif
	pr_info("%s: %s:%s():%d: I've been called\n",
		MODNAME, __FILE__, __FUNCTION__, __LINE__);
	if (p == THE_ONE)
		return secret;
	return 0;
}
EXPORT_SYMBOL(get_skey);

static int __init core_lkm_init(void)
{
	pr_info("%s: inserted\n", MODNAME);
	return 0;
}

static void __exit core_lkm_exit(void)
{
	pr_info("%s: bids you adieu\n", MODNAME);
}

module_init(core_lkm_init);
module_exit(core_lkm_exit);
