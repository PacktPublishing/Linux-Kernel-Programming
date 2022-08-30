# Makefile
# For 'Linux Kernel Programming', Kaiwan N Billimoria, Packt
#  ch9/oom_killer_try
# userspace app.
ALL := oom_killer_try
CC := ${CROSS_COMPILE}gcc

all: ${ALL}
oom_killer_try: oom_killer_try.c
	${CC} -O2 oom_killer_try.c -o oom_killer_try -Wall
oom_killer_try_dbg: oom_killer_try.c
	${CC} -O0 -g -ggdb -DDEBUG oom_killer_try.c -o oom_killer_try_dbg -Wall
clean:
	rm -v -f ${ALL}
