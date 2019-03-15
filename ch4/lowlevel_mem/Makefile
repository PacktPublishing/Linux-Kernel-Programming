# Makefile
# For 'Linux Kernel Development Cookbook', Kaiwan N Billimoria, Packt
#  ch5/lowlevel_mem
#
# To support cross-compiling for kernel modules:
# For architecture (cpu) 'arch', invoke make as:
# make ARCH=<arch> CROSS_COMPILE=<cross-compiler-prefix> 
ifeq ($(ARCH),arm)
    # *UPDATE* 'KDIR' below to point to the ARM Linux kernel source tree on your box
    KDIR ?= ~/rpi_work/rpi_kernel
else ifeq ($(ARCH),powerpc)
    # *UPDATE* 'KDIR' below to point to the PPC64 Linux kernel source tree on your box
    KDIR ?= ~/kernel/linux-4.9.1
else
    # x86[_64]: 'KDIR' is the Linux kernel source tree (headers) on your box
    KDIR ?= /lib/modules/$(shell uname -r)/build
endif

PWD	              := $(shell pwd)
obj-m                 += lowlevel_mem_try.o
lowlevel_mem_try-objs := lowlevel_mem.o ../../klib_lkdc.o
EXTRA_CFLAGS          += -DDEBUG -Wformat=0
  # above, we use -Wformat=0 here to turn Off some printk warnings reg the use
  # of %p instead of %016llx ...; we do so so that we can see the actual virt
  # addr and not a hashed value (kernel security feature); it's ok here, but
  # don't do this in production.
$(info Building for: ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} EXTRA_CFLAGS=${EXTRA_CFLAGS})

all:
	make -C $(KDIR) M=$(PWD) modules
install:
	make -C $(KDIR) M=$(PWD) modules_install
clean:
	make -C $(KDIR) M=$(PWD) clean
