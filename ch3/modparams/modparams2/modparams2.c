/*
 * ch2/modparams/modparams2/modparams2.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch 2: Writing your First Kernel Module-the LKM Framework
 ****************************************************************
 * Brief Description:
 * Same as modparams1, plus, we do some checks: here, we propose that
 * the newly introduced parameter 'control1' is mandatory to be passed
 by the user; this module aborts if it isn't explicityl passed along.
 *
 * For details, please refer the book, Ch 2.
 */
#include <linux/init.h>
#include <linux/module.h>

#define OUR_MODNAME    "modparams2"
MODULE_AUTHOR("<insert your name here>");
MODULE_DESCRIPTION("LKDC book:ch2/modparams/modparams2: module parameters");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/* Module parameters */
static int mp_debug_level;
module_param(mp_debug_level, int, 0660);
MODULE_PARM_DESC(mp_debug_level,
	"Debug level [0-2]; 0 => no debug messages, 2 => high verbosity");

static char *mp_strparam = "My string param";
module_param(mp_strparam, charp, 0660);
MODULE_PARM_DESC(mp_strparam, "A demo string parameter");

static int control1;
module_param(control1, int, 0660);
MODULE_PARM_DESC(control1, "Set to the project's control level [1-5]. MANDATORY");

static int __init modparams1_init(void)
{
	pr_info("%s: inserted\n", OUR_MODNAME);
	if (mp_debug_level > 0)
		pr_info("module parameters passed: "
		"mp_debug_level=%d mp_strparam=%s\ncontrol1=%d\n",
		mp_debug_level, mp_strparam, control1);

	/* param 'control1': if it hasn't been passed (implicit guess), or is
	 * the same old value, or isn't within the right range,
	 * it's Unacceptable!
	 */
	if ((control1 < 1) || (control1 > 5)) {
		pr_warn("%s: Must pass along module parameter"
			" 'control1', value in the range [1-5]; aborting...\n",
			OUR_MODNAME);
		return -EINVAL;
	}
	return 0;	/* success */
}

static void __exit modparams1_exit(void)
{
	if (mp_debug_level > 0)
		pr_info("module parameters passed: mp_debug_level=%d mp_strparam=%s\n",
		mp_debug_level, mp_strparam);
	pr_info("%s: removed\n", OUR_MODNAME);
}

module_init(modparams1_init);
module_exit(modparams1_exit);
