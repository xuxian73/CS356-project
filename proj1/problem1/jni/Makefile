obj-m := pstree.o
KID := ~/kernel/goldfish
CROSS_COMPILE=arm-linux-androideabi-
CC=arm-linux-androideabi-gcc
LD=arm-linux-androideabi-ld

all:
	make -C $(KID) ARCH=arm CROSS_COMPILE=arm-linux-androideabi-  M=$(shell pwd) modules

clean:
	rm -rf *.ko *.o *.mod.c *.order *.symvers
