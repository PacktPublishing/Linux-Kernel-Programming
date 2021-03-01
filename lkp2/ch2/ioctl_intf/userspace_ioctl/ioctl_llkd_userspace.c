/*
 * user_kernel_comm/ioctl_intf/userspace_ioctl/ioctl_llkd_userspace.c
 **************************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Chapter - User-Kernel communication pathways
 **************************************************************************
 * Brief Description:
 * User space app to test interfacing between user and kernel address spaces
 * via the ioctl.
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

	// 3. Toggle it's power status
	if (0 == power) {
		printf("%s: Device OFF, powering it On now ...\n", argv[0]);
		if (ioctl(fd, IOCTL_LLKD_IOCSPOWER, 1) == -1) {
			perror("ioctl IOCTL_LLKD_IOCSPOWER failed");
			close(fd);
			exit(EXIT_FAILURE);
		}
		printf("%s: power is ON now.\n", argv[0]);
	} else if (1 == power) {
		printf("%s: Device ON, powering it OFF in 3s ...\n", argv[0]);
		sleep(3);	/* yes, careful here of sleep & signals! */
		if (ioctl(fd, IOCTL_LLKD_IOCSPOWER, 0) == -1) {
			perror("ioctl IOCTL_LLKD_IOCSPOWER failed");
			close(fd);
			exit(EXIT_FAILURE);
		}
		printf("%s: power OFF ok, exiting..\n", argv[0]);
	}

	close(fd);
	exit(EXIT_SUCCESS);
}
