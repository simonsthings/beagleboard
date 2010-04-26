###################################################
#
# Makefile for building the hellokernel derived from char-driver sample module
#
###################################################

KERNEL_VERSION := `2.6.29-r46`
KERNEL_DIR := /opt/oe/angstrom-dev/work/beagleboard-angstrom-linux-gnueabi/linux-omap-2.6.29-r46/git
KERNEL_CROSS := /opt/oe/angstrom-dev/cross/armv7a/bin/arm-angstrom-linux-gnueabi-

ifeq ($(DEBUG),y)
	EXTRA_CFLAGS += -O -g # -O is need to expand inlines
else
	EXTRA_CFLAGS += -O2
endif

ifneq ($(KERNELRELEASE),)
# call from kernel build system
obj-m := helloMcBSP.o
else
PWD := $(shell pwd)

default: kernel-module
.PHONY: kernel-module

TARGET_ARCH = "arm"
TARGET_OS = "linux-gnueabi"

kernel-module:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(KERNEL_CROSS) modules
endif

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions sample Module.symvers

depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
