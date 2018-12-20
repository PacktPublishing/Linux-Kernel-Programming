# ch2/cross/Makefile: To support cross-compiling for the ARM:
# Invoke as :
#  make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- 
ifeq ($(ARCH),arm)
    # *UPDATE* 'KDIR' below to point to the ARM Linux kernel 
    # source tree on your box
    KDIR ?= ~/kernel/4.14.52_arm   # for example
else ifeq ($(ARCH),powerpc)
    # *UPDATE* 'KDIR' below to point to the PPC64 Linux kernel
    # source tree on your box
    KDIR ?= ~/kernel/linux-4.9.1   # for example
else   # it's x86, the 'default'
    KDIR ?= /lib/modules/$(shell uname -r)/build 
endif

obj-m        += hello.o
EXTRA_CFLAGS += -DDEBUG
$(info Building for: ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} EXTRA_CFLAGS=${EXTRA_CFLAGS})

all:
	make -C $(KDIR) M=$(PWD) modules
install:
	make -C $(KDIR) M=$(PWD) modules_install
clean:
	make -C $(KDIR) M=$(PWD) clean
