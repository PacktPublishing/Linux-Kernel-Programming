/*
 * sed_common.h
 *
 * Common header for both the userapp_sed1.c user space app and the
 * sed_drv kernel module.
 */

/* The 'magic' number for our driver; see
 * Documentation/ioctl/ioctl-number.rst
 * Of course, we don't know for _sure_ if the magic # we choose here this
 * will remain free; it really doesn't matter, this is just for demo purposes;
 * don't try and upstream this without further investigation :-)
 */
#define IOCTL_LLKD_SED_MAGIC		0xA9
#define	IOCTL_LLKD_SED_MAXIOCTL		2
/*
 * The _IO{R|W}() macros can be summarized as follows:
_IO(type,nr)                  ioctl command with no argument
_IOR(type,nr,datatype)        ioctl command for reading data from the kernel/drv
_IOW(type,nr,datatype)        ioctl command for writing data to the kernel/drv
_IOWR(type,nr,datatype)       ioctl command for read/write transfers
*/
/* our ioctl (IOC) RESET command */
//#define IOCTL_LLKD_KTIMR_IOCRESET		_IO(IOCTL_LLKD_KTIMR_MAGIC, 0)

/* our ioctl (IOC) encrypt message command */
#define IOCTL_LLKD_SED_IOC_ENCRYPT_MSG		_IOR(IOCTL_LLKD_SED_MAGIC, 1, int)

/* our ioctl (IOC) decrypt message command */
#define IOCTL_LLKD_SED_IOC_DECRYPT_MSG		_IOR(IOCTL_LLKD_SED_MAGIC, 2, int)

/* Metadata structure for the 'payload' */
#define MAX_DATA	512

typedef unsigned char u8;
struct sed_ds {
	int data_xform; // data transform to apply to the data payload
	int len;		// length of data payload (bytes)
	int timed_out;	// 1 if the op timed out
	char data[MAX_DATA];	// the payload
};
// Data transformations
enum xform { XF_NONE, XF_DECRYPT, XF_ENCRYPT };

#define SHOW_TIME()		do {		\
	pr_debug("%lld ns", ktime_get_real_ns()); \
} while (0)

// t1, t2: nanosecond-accurate timestamps
#define SHOW_DELTA(t1, t2)	do {	\
	if (time_after((unsigned long)t2, (unsigned long)t1)) {	\
		pr_debug("delta: %lld ns (= %lld us = %lld ms)\n",	\
		(t2) - (t1),	\
		((t2) - (t1))/1000,	\
		((t2) - (t1))/1000000	\
		); \
	} else	\
		pr_debug("SHOW_DELTA(): *invalid* t2 < t1?\n");	\
} while (0)
