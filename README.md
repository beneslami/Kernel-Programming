# Conecpts behind netlink sockets

Netlink socket is a kernel feature which is provided in the linux kernel and it is a feature which is used for IPC between Userspace and Kernel space.
Kernel modules are pieces of codes that can be loaded and unloaded into the kernel upon demand. 

![diagram](img.png)

They extend the funcitonality of the kernel without the need to reboot the system or recompiling the linux kernel.
```
* Load the kernel module: sudo insmod <lkm.ko>
* unload lkm: rmmod <lkm>
* check loaded lkm: lsmod
```

How to write a kernel module ? first and foremost, we need to add a proper header file into our code base.
```
#include <linux/module.h>
```

Each module must have initiator and cleanup functions. The initiator must be invoked when the module is called and the cleanup is called also when the module is being removed.
there are two main functions for declaring initiator and cleanup:
```
module_init()
module_exit()
```

If we don't use the init and cleanup modules, the kernel calles default init and cleanup functions.
Also, it is important to mention the auther and description of the module according to below format:

```
MODULE_AUTHOR(author_name);
MODULE_DESCRIPTION(module_desc);
```

The format for writing Makefile is as below:
```
obj-m += helloworldLKM.o
all:
	make -C /lib/modules/`uname -r` /build M=$(PWD) modules
clean:
	make -C /lib/modules/`uname -r` /build M=$(PWD) clean
```
To instantly monitor kernel module behavior, simply run below command:
```
tail -f /var/log/kern.log
```

# Netlink
According to below picture, accessing to hardware resources is via linux modules or device drivers. And
also userspace applications can indirectly access to hardware resources via kernel. There are 4 means of communicating
with linux kernel: 
1. Netlink socket
2. IOCTLS
3. Device files
4. system calls

![diagram](arch.png)

A socket based technique for communicating between userspace and kernel space was developed to build the **unified interface**
using which userspace applications can interact with **various kernel subsystems**. Any communication messages between two parties
must follow pre-defined message format. The format for Netlink messages is as below:

![diagram](nlmsg.png)

and the header file must contain a data structure as below:
```
struct nlmsghdr{
    u32 nlmsg_len;
    u16 nlmsg_type;
    u16 nlmsg_flags;
    u32 nlmsg_seq;
    u32 nlmsg_pid;
};
```
According to below picture, there are five field in the netlink socket header which they are being elaborated in 
detail:

![diagram](nlmsghdr.png)

* As said before, **nlmsg_len** represents the total length of netlink message including the payload.
* The second type is **nlmsg_type**. There are 4 standard types defined in ```/usr/include/linux/netlink.h```.
```asm
#define NLMSG_NOOP  0x1
#define NLMSG_ERROR 0x2
#define NLMSG_DONE 0x3
#define NLMSG_OVERRUN 0x4
```
NLMSG_NOOP: When the other party receives this message, it does nothing except it replies with NLMSG_DONE telling the sender
that all is fine.

NLMSG_ERROR: When the other party receives this message as a reply to the message sent previously, it means that other party
failed to perform requested action (negative feedback)

NLMSG_DONE: This is the last Netlink message in the cascade of multiple Netlink message units.

NLMSG_OVERRUN: Currently not used in linux kernel anywhere.

Besides above, User can define his own message types which should be greater than 16

* The third item is **nlmsg_flags**. These flags are set in Netlink message to convey additional information to the recipient.
It is worth mentioning that multiple flags could be set using bitwise ADN/OR operators. These flags also cen be found in the same
  header as before.
  
* The third argument is **nlmsg_seq**. When the userspace application sends a Netlink request to kernel space, it must set
a unique number to this request if the user space application sets NLM_F_ACC flag. When the kernel space replies back with
  confirmation message to user space application, it sets the same sequence number which was specified in the request message
  sent from the application. This helps the user space application to correlate which Netlink reply is for which netlink request
  in case the application has issued multiple netlink requests to the kernel and awaiting reply.
  
* PID or Port ID. It is set by the Userspace application while sending netlink message to the kernel space. Since it should 
be unique to the application, therefore it's good to assign process id to that. Note that, this value is set to zero for Netlink
  messages originating from kernel space to userspace. 

User space program can decide to communicate with any kernel module sub-systems running in the linux kernel. 
So the question is that how the userspace program can identify that with which linux kernel sub-systems they want
to communicate ? The simple answer is that every linux kernel sub-system is assigned a unique ID. This ID is called
Netlink Protocol Number which is assigned to each Netlink capable kernel sub-system. There are reserved ID numbers for 
important kernel sub-system modules such as TCP/IP stack etc. which can be found in the file linux/netlink.h

When the kernel space receives data from User space application via Netlink, data is received in a data structure called 
**socket Buffer**, which is defined in include/linux/skbuff.h

Kernel uses this data structure extensively for multiple purposes:
1. For transferring messages from one kernel subsystem to another
2. for receiving Network packet
3. packet movement upwards and downwards in the layers of TCP/IP stack


Note that in the userspace application, there are two threads operating in parallel:
1. main thread which independently sends data to the linux kernel module
2. child thread which independently receives data from the linux kernel module