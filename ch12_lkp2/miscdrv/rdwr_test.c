/*
 * ch12/miscdrv/rdwr_test.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 12 : Writing a Simple Misc Device Driver
 ****************************************************************
 * Brief Description:
 * USER SPACE app : a generic read-write test bed for demo drivers.
 * THis simple user mode app allows you to issue a read or a write
 * to a specified device file.
 *
 * For details, please refer the book, Ch 12.
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define READ_OPT	'r'
#define WRITE_OPT	'w'

static int stay_alive;

static inline void usage(char *prg)
{
	fprintf(stderr, "Usage: %s r|w device_file num_bytes\n"
		" 'r' => read num_bytes bytes from the device node device_file\n"
		" 'w' => write num_bytes bytes to the device node device_file\n", prg);
}

int main(int argc, char **argv)
{
	int fd, flags = O_RDONLY;
	ssize_t n;
	char opt, *buf = NULL;
	size_t num = 0;

	if (argc != 4) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	opt = argv[1][0];
	if (opt != 'r' && opt != 'w') {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	if (opt == WRITE_OPT)
		flags = O_WRONLY;

	fd = open(argv[2], flags, 0);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}
	printf("Device file \"%s\" opened (in %s mode): fd=%d\n",
	       argv[2], (flags == O_RDONLY ? "read-only" : "write-only"), fd);

	num = atoi(argv[3]);
	/* if ((num < 0) || (num > INT_MAX)) { */
	/* FYI, for the above line of code, the cppcheck(1) tool via the Makefile's
	 * 'sa' target caught this:
	 * "style: The unsigned expression 'num' will never be negative so it is either
	 * pointless or an error to check if it is. [unsignedLessThanZero]"
	 */
	if (num > INT_MAX) {
		fprintf(stderr, "%s: number of bytes '%zu' invalid.\n", argv[0], num);
		close(fd);
		exit(EXIT_FAILURE);
	}

	buf = calloc(num, 1);
	if (!buf) {
		fprintf(stderr, "%s: out of memory!\n", argv[0]);
		close(fd);
		exit(EXIT_FAILURE);
	}

	if (opt == READ_OPT) {	// test reading..
		n = read(fd, buf, num);
		if (n < 0) {
			perror("read failed");
			fprintf(stderr, "Tip: see kernel log\n");
			free(buf);
			close(fd);
			exit(EXIT_FAILURE);
		}
		printf("%s: read %zd bytes from %s\n", argv[0], n, argv[2]);
		printf(" Data read:\n\"%.*s\"\n", (int)n, buf);

#if 0
		/* Test the lseek; typically, it should fail */
		off_t ret = lseek(fd, 100, SEEK_CUR);
		if (ret == (off_t)-1)
			fprintf(stderr, "%s: lseek on device failed\n", argv[0]);
#endif
	} else {		// test writing ..
		n = write(fd, buf, num);
		if (n < 0) {
			perror("write failed");
			fprintf(stderr, "Tip: see kernel log\n");
			free(buf);
			close(fd);
			exit(EXIT_FAILURE);
		}
		printf("%s: wrote %ld bytes to %s\n", argv[0], n, argv[2]);
	}

	if (stay_alive == 1) {
		printf("%s:%d: stayin' alive (in pause()) ...\n", argv[0], getpid());
		pause();	/* block until a signal is received */
	}

	free(buf);
	close(fd);
	exit(EXIT_SUCCESS);
}
