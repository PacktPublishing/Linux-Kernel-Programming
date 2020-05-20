# Makefile
ALL :=  helloworld helloworld_dbg

CC=${CROSS_COMPILE}gcc
CFLAGS=-Wall -UDEBUG
CFLAGS_DBG=-g -ggdb -gdwarf-4 -O0 -Wall -Wextra -DDEBUG

all: ${ALL}

helloworld: helloworld.c
	${CC} ${CFLAGS} helloworld.c -o helloworld
	${CROSS_COMPILE}strip --strip-all helloworld
helloworld_dbg: helloworld.c
	${CC} ${CFLAGS_DBG} helloworld.c -o helloworld_dbg

clean:
	rm -vf ${ALL} core* vgcore* *.o *~ *.c.gcov *.gcda *.gcno
