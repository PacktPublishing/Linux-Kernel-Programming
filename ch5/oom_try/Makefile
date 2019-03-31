# Makefile
# For 'Linux Kernel Development Cookbook', Kaiwan N Billimoria, Packt
#  ch5/oom_try
# userspace app.
ALL := oom-killer-try rdpg_oom_try
CC := ${CROSS_COMPILE}gcc

all: ${ALL}
oom-killer-try: oom-killer-try.c
	${CC} -O2 oom-killer-try.c -o oom-killer-try -Wall
rdpg_oom_try: rdpg_oom_try.c
	${CC} -O2 rdpg_oom_try.c -o rdpg_oom_try -Wall
clean:
	rm -v -f ${ALL}
