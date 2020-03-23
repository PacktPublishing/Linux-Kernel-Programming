# Makefile for netlink userspace
ALL := netlink_userapp netlink_userapp_dbg
all: ${ALL}

netlink_userapp: netlink_userapp.c
	${CROSS_COMPILE}gcc -O2 netlink_userapp.c -o netlink_userapp -Wall -Wextra
netlink_userapp_dbg: netlink_userapp.c
	${CROSS_COMPILE}gcc -O0 -g -ggdb netlink_userapp.c -o netlink_userapp_dbg -Wall -Wextra
clean:
	rm -fv ${ALL}
