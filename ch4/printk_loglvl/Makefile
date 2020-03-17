# ch4/printk_loglvl/Makefile
# Simplest kernel module Makefile

PWD          := $(shell pwd)
obj-m        += printk_loglvl.o

# Enable the pr_debug() as well (rm the comment from one of the lines below)
#EXTRA_CFLAGS += -DDEBUG
#CFLAGS_printk_loglvl.o := -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
install:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules_install
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
