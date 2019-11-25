# ch5/modstacking/Makefile
#
# To cross compile our simple kernel module for the Raspberry Pi (an ARM-based
# system), invoke the build like this:
#  make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
# (We expect that the toolchain is installed and in the PATH, and the env
# var RPI_STG is set to the 'staging folder' containing all the R Pi stuff).

ifeq ($(ARCH),arm)
    # *UPDATE* 'KDIR' below to point to the ARM Linux kernel 
    # source tree on your box
    KDIR ?= ~/rpi_work/kernel_rpi  # the R Pi kernel
else ifeq ($(ARCH),powerpc)
    # *UPDATE* 'KDIR' below to point to the PPC64 Linux kernel
    # source tree on your box
    KDIR ?= ~/kernel/linux-4.9.1   # for example
else   # it's x86, the 'default'
    KDIR ?= /lib/modules/$(shell uname -r)/build 
endif

PWD          := $(shell pwd)
obj-m        += core_lkm.o
obj-m        += user_lkm.o

EXTRA_CFLAGS += -DDEBUG
$(info Building for: ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} EXTRA_CFLAGS=${EXTRA_CFLAGS})

all:
	make -C $(KDIR) M=$(PWD) modules
install:
	make -C $(KDIR) M=$(PWD) modules_install
clean:
	make -C $(KDIR) M=$(PWD) clean
