ccflags-y := -DDEBUG -O1 -g

PWD := $(shell pwd)
INCLUDE := include
SRC := src

KERNELDIR ?= $(PWD)/kernel/linux-4.9.83
EXTRA_CFLAGS := -I$(INCLUDE)


obj-m := filesystems.o 
filesystems-objs := $(SRC)/filesystems.o $(SRC)/general_functions.o $(SRC)/file_functions.o $(SRC)/SuperBlock_functions.o $(SRC)/directory_functions.o $(SRC)/inodes_functions.o $(SRC)/persistence_functions.o $(SRC)/statfs_function.o


all :
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
