# ch2/lkm1/Makefile
# Simplest kernel module Makefile

PWD   := $(shell pwd)
obj-m += hello.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
install:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules_install
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
