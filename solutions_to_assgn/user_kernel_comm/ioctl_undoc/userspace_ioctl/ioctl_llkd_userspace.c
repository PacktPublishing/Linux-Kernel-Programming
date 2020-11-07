/*
 * solutions_to_assgn/ch12/ioctl_undoc/userspace_ioctl/ioctl_llkd_userspace.c
 **************************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 12 : User - kernel communication pathways
 **************************************************************************
 * Assignment : ch12: ioctl #2: 'ioctl_undoc':
 *
 * Using the provided ch12/ioctl_intf/ code as a template, write a userspace
 * 'C' application and a kernel-space (char) device driver implementing the
 * ioctl method. Add in a 'driver context' data structure (that we use in
 * several examples), allocate and initialize it. Now, in addition to the
 * earlier three ioctl 'commands' we use, setup a fourth 'undocumented' command
 * (you can call it IOCTL_LLKD_IOCQDRVSTAT). It's job: when queried from
 * userspace via ioctl(2), it must return the contents of the 'driver context'
 * data structure to userspace; the userspace 'C' app must print out the
 * content.
 *
 * For details refer the book, Ch 12.
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "../ioctl_llkd.h"

int main(int argc, char **argv)
{
	int fd, power;
	struct drv_ctx *drvctx = NULL;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s device_file\n\
  If device_file does not exist, create it using mknod(1) (as root)\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if ((fd = open(argv[1], O_RDWR, 0)) == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}
	printf("device opened: fd=%d\n", fd);

	printf
	    ("(FYI, IOCTL_LLKD_IOCRESET = 0x%x IOCTL_LLKD_IOCQPOWER= 0x%x "
	     "IOCTL_LLKD_IOCSPOWER=0x%x)\n",
	     IOCTL_LLKD_IOCRESET, (unsigned int)IOCTL_LLKD_IOCQPOWER,
	     (unsigned int)IOCTL_LLKD_IOCSPOWER);

	// Test our various ioctl's ...
	// 1. Reset the device
	if (ioctl(fd, IOCTL_LLKD_IOCRESET, 0) == -1) {
		perror("ioctl IOCTL_LLKD_IOCRESET failed");
		close(fd);
		exit(EXIT_FAILURE);
	}
	printf("%s: device reset.\n", argv[0]);

	// 2. Query it's power status
	if (ioctl(fd, IOCTL_LLKD_IOCQPOWER, &power) == -1) {
		perror("ioctl IOCTL_LLKD_IOCQPOWER failed");
		close(fd);
		exit(EXIT_FAILURE);
	}
	printf("%s: power=%d\n", argv[0], power);

	// 3. Run the 'undocumented' ioctl !
	drvctx = calloc(1, sizeof(struct drv_ctx));
	if (!drvctx) {
		fprintf(stderr, "%s: calloc failed\n", argv[0]);
		close(fd);
		exit(EXIT_FAILURE);
	}
//	drvctx->power = 1;
	printf("%s: running the 'undocumented' ioctl cmd IOCQDRVSTAT now...\n", argv[0]);
	if (ioctl(fd, IOCTL_LLKD_IOCQDRVSTAT, drvctx) == -1) {
		perror("ioctl IOCTL_LLKD_IOCSPOWER failed");
		free(drvctx);
		close(fd);
		exit(EXIT_FAILURE);
	}
	printf("%s: 'driver context' details:\n"
	" power = %d\n"
	" tx = %d, rx = %d\n"
		, argv[0], drvctx->power, drvctx->tx, drvctx->rx);

	free(drvctx);
	close(fd);
	exit(EXIT_SUCCESS);
}
