ifneq ($(KERNELRELEASE),)
	obj-m := drive.o 
else
	KERNELDIR ?=/lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
c:
	rm -rf *~ *.o *.lo *.ko *.mod *.mod.c .*.cmd modules.order Module.markers Module.symvers .tmo_versions drive.
endif
