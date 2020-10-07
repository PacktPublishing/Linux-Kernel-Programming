/*
 * ch7/userspc_cpuaffinity/userspc_cpuaffinity.c
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

static unsigned int numcores;

static inline void print_ruler(unsigned int len)
{
	int i;

	printf("       +");
	for (i=1; i<=len*3; i++) {
		if ((i%3) == 0)
			printf("+");
		else
			printf("-");
	}
	printf("\n");
}

/* disp_cpumask():
 * Print the provided CPU bitmask @cpumask (along with the 'ruler' lines),
 * for a max of @ncores-1 CPU cores, in (a more intuitive) right-to-left order.
 */
static void disp_cpumask(pid_t pid, cpu_set_t *cpumask, unsigned int ncores)
{
	int i;
	char tmpbuf[128];

	printf("CPU affinity mask for PID %d:\n", pid);
	snprintf(tmpbuf, 127, "ps aux |awk '$2 == %d {print $0}'", pid);
	if (system(tmpbuf) == -1)
		fprintf(stderr, "Warning: %s():system(3) (to show ps output)"
			" failed\n", __func__);

	print_ruler(ncores);

	printf("core#  |");
	for (i=ncores-1; i>=0; i--)
		printf("%2d|", i);
	printf("\n");
	print_ruler(ncores);

	printf("cpumask|");
	for (i=ncores-1; i>=0; i--)
		printf("%2u|", CPU_ISSET(i, cpumask));
	printf("\n");
	print_ruler(ncores);
}

static int query_cpu_affinity(pid_t pid)
{
	cpu_set_t cpumask;

	CPU_ZERO(&cpumask);
	if (sched_getaffinity(pid, sizeof(cpu_set_t), &cpumask) < 0) {
		perror("sched_getaffinity() failed");
		return -1;
	}
	disp_cpumask(pid, &cpumask, numcores);

	return 0;
}

static int set_cpu_affinity(pid_t pid, unsigned long bitmask)
{
	cpu_set_t cpumask;
	int i;

	CPU_ZERO(&cpumask);

	/* Iterate over the given bitmask, setting CPU bits as required */
	for (i=0; i<sizeof(unsigned long)*8; i++) {
		/* printf("bit %d: %d\n", i, (bitmask >> i) & 1); */
		if ((bitmask >> i) & 1)
			CPU_SET(i, &cpumask);
	}

	if (sched_setaffinity(pid, sizeof(cpu_set_t), &cpumask) < 0) {
		perror("sched_setaffinity() failed");
		return -1;
	}
	disp_cpumask(pid, &cpumask, numcores);

	return 0;
}

int main (int argc, char **argv)
{
	pid_t pid = getpid();
	unsigned long new_cpumask = 0x0;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s number-of-CPU-cores "
		 "[PID] [new-CPU-mask]\n"
		"(If using the optional params, you must at least pass"
		" the process PID;\nwe (attempt to) set CPU affinity only if"
		" new-CPU-mask is passed)\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	numcores = atoi(argv[1]);
	if (argc >= 3)
		pid = atoi(argv[2]);

	query_cpu_affinity(pid);
	if (argc == 4) {
		new_cpumask = strtoul(argv[3], 0, 0);
		set_cpu_affinity(pid, new_cpumask);
	}

	exit(EXIT_SUCCESS);
}
