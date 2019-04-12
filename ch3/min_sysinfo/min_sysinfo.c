/*
 * ch3/min_sysinfo/min_sysinfo.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch 3: Writing your First Kernel Module - LKMs Part 2
 ****************************************************************
 * Brief Description:
 * Make use of some convenience macros provided by the kernel to glean and
 * print some minimal CPU, OS details. Can be cross-compiled and tried out
 * on various Linux systems.
 *
 * For details, please refer the book, Ch 3.
 */
#include <linux/init.h>
#include <linux/module.h>

#define MYMODNAME   "min_sysinfo"
MODULE_AUTHOR("<insert your name here>");
MODULE_DESCRIPTION
    ("LKDC book:ch3/min_sysinfo: print some minimal system info");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/* lkdc_sysinfo2:
 * A more security-aware version of the lkdc_sysinfo routine. We used
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

	/* Word ranges: min & max: defines are in include/linux/kernel.h */
	pr_info("Word [U|S][8|16|32|64] ranges: unsigned max, signed max, signed min:\n"
		" U8_MAX = %20u,  S8_MAX = %20d,  S8_MIN = %20d\n"
		"U16_MAX = %20u, S16_MAX = %20d, S16_MIN = %20d\n"
		"U32_MAX = %20u, S32_MAX = %20d, S32_MIN = %20d\n"
		"U64_MAX = %20llu, S64_MAX = %20lld, S64_MIN = %20lld\n"
		"PHYS_ADDR_MAX = %llu\n",
			U8_MAX, S8_MAX, S8_MIN,
			U16_MAX, S16_MAX, S16_MIN,
			U32_MAX, S32_MAX, S32_MIN,
			U64_MAX, S64_MAX, S64_MIN,
			PHYS_ADDR_MAX);
}
EXPORT_SYMBOL(lkdc_sysinfo2);

static void lkdc_sysinfo(void)
{
	char msg[128];

	memset(msg, 0, strlen(msg));
	snprintf(msg, 47, "%s(): minimal Platform Info:\nCPU: ", __func__);

	/* Strictly speaking, all this #if... is considered ugly and should be
	   isolated as far as is possible */
#ifdef CONFIG_X86
#if(BITS_PER_LONG == 32)
	strncat(msg, "x86-32, ", 9);
#else
	strncat(msg, "x86_64, ", 9);
#endif
#endif
#ifdef CONFIG_ARM
	strncat(msg, "ARM-32, ", 9);
#endif
#ifdef CONFIG_ARM64
	strncat(msg, "Aarch64, ", 10);
#endif
#ifdef CONFIG_MIPS
	strncat(msg, "MIPS, ", 7);
#endif
#ifdef CONFIG_PPC
	strncat(msg, "PowerPC, ", 10);
#endif
#ifdef CONFIG_S390
	strncat(msg, "IBM S390, ", 11);
#endif

#ifdef __BIG_ENDIAN
	strncat(msg, "big-endian; ", 13);
#else
	strncat(msg, "little-endian; ", 16);
#endif

#if(BITS_PER_LONG == 32)
	strncat(msg, "32-bit OS.\n", 12);
#elif(BITS_PER_LONG == 64)
	strncat(msg, "64-bit OS.\n", 12);
#endif
	pr_info("%s", msg);
}
EXPORT_SYMBOL(lkdc_sysinfo);

static int __init min_sysinfo_init(void)
{
	pr_info("%s: inserted\n", MYMODNAME);
	lkdc_sysinfo();
	lkdc_sysinfo2();
	return 0;	/* success */
}

static void __exit min_sysinfo_exit(void)
{
	pr_info("%s: removed\n", MYMODNAME);
}

module_init(min_sysinfo_init);
module_exit(min_sysinfo_exit);
