/*
 * ch6/ebpf_stacktrace/helloworld.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Programming"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Programming
 *
 * From: Ch 6: Kernel Internals Essentials - Processes and Threads
 ****************************************************************
 * Brief Description:
 * A quick user mode 'hello, world' with the caveat that it runs in an infinite
 * loop! Why? So that we can trace something menaingful with the modern eBPF
 * BCC stackcount-bpfcc tool!
 * 
 * For details, please refer the book, Ch 6.
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
	while (1) {
		printf("Hello, world! PID = %d\n", getpid());
		usleep(10000);  // sleep for 10 ms
	}
	exit(EXIT_SUCCESS);
}
