# Makefile - usbdpfp kernel module build file for Linux kernel 2.6
#
# Copyright 1996-2011 DigitalPersona, Inc.  All rights reserved.
#  
# see: linux/Documentation/kbuild/makefiles.txt for full documentation on makefile
# see: linux/Documentation/kbuild/modules.txt for full documenttion on kbuild
#

DRIVER_VERSION	:= 2.0.0.7

KDIR ?= /lib/modules/$(shell uname -r)/build 
PWD	 := $(shell pwd)
OBJ	 := mod_usbdpfp

obj-m	:= $(OBJ).o
$(OBJ)-objs	:= usbdpfp.o 

EXTRA_CFLAGS	:= -DDRIVER_VERSION=\"v$(DRIVER_VERSION)\"

all:	clean compile

compile:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

load:	
	su -c "insmod ./mod_usbdpfp.ko"

load_debug:	
	@echo "try \"tail -f /var/log/messages\" in another window as root...";
	su -c "insmod ./mod_usbdpfp.ko debug=1"

load_mdebug:
	su -c "insmod ./mod_usbdpfp.ko mdebug=1"

unload:
	-su -c "rmmod -s mod_usbdpfp"

clean:
	rm -fr $(OBJ).o $(OBJ).ko $(OBJ).*.* .$(OBJ).* .tmp_versions* .*.cmd *.o *~ *.order *.symvers

debug: unload clean compile load_debug
	tail -f /var/log/messages

