//
// Created by Ben on 3/6/22.
//
#include <linux/module.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/string.h>

#define NETLINK_TEST_PROTOCOL 31
static struct sock *nl_sk = NULL;

static void netlink_recv_msg_fn(struct sk_buff *skb_in){
    struct nlmsdgdr *nlh_recv, *nlk_reply;
    int user_space_port_id;
    char *user_space_data;
    int user_space_data_len;
    char kernel_reply[256];
    struct sk_buff *skb_out;
    printk(KERN_INFO "%s invoked", __FUNCTION__ );
    nlk_recv = (struct nlmsghdr*)(skb_in->data);
    //nlmsg_dump(nlh_recv);
    user_space_process_port_id = nlh_recv->nlmsg_pid;
    printk(KERN_INFO "%s(%d): port id of the sending user space process = %u\n", __FUNCTION__ , __LINE__, user_space_process_port_id);
    user_space_data = (char*)nlmsg_data(nlh_recv);
    user_space_data_len = skb_in->len;
    printk(KERN_INFO "%s(%d) : message received from userspace = %s ", __FUNCTION__);
    if(nlh_recv->nlmsg_flags & NLM_F_ACK){
        memset(kernel_reply, 0, sizeof(kernel_reply));
        snprintf(kernel_reply, sizeof kernel_reply, "MSG from process %d has been processed by kernel", nlh_recv->nlmsg_pid);
        skb_out = nlmsg_new(sizeof(kernel_reply), 0);
        nlh_reply = nlmsg_put(skb_out,
                              0,
                              nlh_recv->nlmsg_seq,
                              NLMSG_DONE,
                              sizeof(kernel_reply),
                              0
                              );
        strncpy(nlmsg_data(nlh_reply), kernel_reply, sizeof(kernel_reply));
        res = nlmsg_unicast(nl_sk, skb_out, user_space_port_id);
        if(res < 0){
            printk(KERN_INFO "Error while sendingg the data back to userspace\n");
            kfree_skb(skb_out);
        }
    }
}

static struct netlink_kernel_cfg cfg = {
        .input = netlink_recv_msg_fn,
};

static int __init NetlinkGreetings_init(void){
    printk(KERN_INFO "Hello kernel, I am kernel module \n");
    nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST_PROTOCOL, &cfg);
    if(!nl_sk){
        printk(KERN_INFO "kernel netlink socket for netlink protocol %u failed\n", NETLINK_TEST_PROTOCOL);
        return _ENOMEM;
    }
    printk(KERN_INFO, "Netlink socket created successfully");
    return 0;
}

static void __exit NetlinkGreetings_exit(void){
    printk(KERN_INFO "Byte byte. Exiting\n");
    netlink_kernel_release(nl_sk);
    nl_sk = NULL;
}

moduel_init(NetlinkGreetings_init);
module_exit(NetlinkGreetings_exit);

MODULE_AUTHOR("BEN ESLAMI");
MODULE_DESCRIPTION("Netlink Example");
MODULE_LICENSE("GPL");