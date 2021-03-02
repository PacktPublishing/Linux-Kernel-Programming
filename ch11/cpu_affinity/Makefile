# ch11/Makefile
# ***************************************************************
# * This program is part of the source code released for the book
# *  "Linux Kernel Programming"
# *  (c) Author: Kaiwan N Billimoria
# *  Publisher:  Packt
# *  GitHub repository:
# *  https://github.com/PacktPublishing/Linux-Kernel-Programming
# ***************************************************************
# * From: Ch 11 : CPU Scheduling, Part 2
# ***************************************************************
ALL := userspc_cpuaffinity userspc_cpuaffinity_dbg
all: ${ALL}
userspc_cpuaffinity: userspc_cpuaffinity.c  # the userspace app
	gcc -Wall -Os userspc_cpuaffinity.c -o userspc_cpuaffinity
userspc_cpuaffinity_dbg: userspc_cpuaffinity.c  # the userspace app
	gcc -g -ggdb -Wall -O0 userspc_cpuaffinity.c -o userspc_cpuaffinity_dbg
clean:
	rm -fv ${ALL}
