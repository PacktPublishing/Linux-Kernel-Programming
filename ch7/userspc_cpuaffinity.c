/*
 * ch7/s1/s1.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Linux Kernel Development Cookbook"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Linux-Kernel-Development-Cookbook
 *
 * From: Ch 7 : CPU Scheduling
 ****************************************************************
 * Brief Description:
 *
 * For details, please refer the book, Ch 7.
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sched.h>

static void disp_cpumask(cpu_set_t *cpumask)
{
		int i;

		printf("CPU core# : ");
		for (i=0; i<CPU_COUNT(cpumask); i++)
				printf("%d", i);
		printf("\n            ");
		for (i=0; i<CPU_COUNT(cpumask); i++) {
				printf("%u", CPU_ISSET(i, cpumask));
		}
		printf("\n");
}

static int query_sched(void)
{
		cpu_set_t cpumask;

		CPU_ZERO(&cpumask);
		if (sched_getaffinity(0, sizeof(cpu_set_t), &cpumask) < 0) {
				perror("sched_getaffinity() failed");
				return -1;
		}
		disp_cpumask(&cpumask);

		return 0;
}

int main (int argc, char **argv)
{
/*	if (argc < 2) {
		fprintf(stderr, "Usage: %s \n", argv[0]);
		exit(EXIT_FAILURE);
	}
*/
		query_sched();

		exit(EXIT_SUCCESS);
}
