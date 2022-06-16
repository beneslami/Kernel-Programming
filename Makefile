obj-m += MyModule.o
all:
        make -C /lib/modules/`uname -r`/build M=$(PWD) modules
        gcc -g userspace.c -o userspace -lpthread
clean:
        make -C /lib/modules/`uname -r`/build M=$(PWD) clean