ccflags-y := -DDEBUG -O1 -g



KERNELDIR ?= ../kernel/linux-4.9.83
PWD := $(shell pwd)

obj-m := filesystems.o 


all :
	$(info obj-m : $(obj-m))
	$(info obj-m : $(filesystems.objs))
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
