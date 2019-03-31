# Makefile
# For 'Linux Kernel Development Cookbook', Kaiwan N Billimoria, Packt
#  ch5/oom_try
# userspace app.
ALL := oom-killer-try
CC := ${CROSS_COMPILE}gcc

all: ${ALL}
oom-killer-try: oom-killer-try.c
	${CC} -O2 oom-killer-try.c -o oom-killer-try -Wall
clean:
	rm -v -f ${ALL}
