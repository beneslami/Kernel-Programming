#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
#include <memory.h>
#include <stdint.h>
#include <pthread.h>
#include "netLinkUtils.h"

int send_netlink_msg_to_kernel(int sock_fd, char *msg, uint32_t msg_size, int nlmsg_type, uint16_t flags);

int send_netlink_msg_to_kernel(int sock_fd, char *msg, uint32_t msg_size, int nlmsg_type, uint16_t flags){
    struct sockaddr_nl dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; // destination is kernel

    struct nlmsghdr *nlh = (struct nlmsghdr *)calloc(1, NLMSG_HDRLEN + NLMSG_SPACE(msg_size));
    nlh->nlmsg_len = NLMSG_HDRLEN + NLMSG_SPACE(msg_size);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flag = flags;
    nlh->nlmsg_type = nlmsg_type;
    nlh->nlmsg_seq = 0;
    strncpy(NLMSG_DATA(nlh), msg, msg_size);

    struct iovec iov;
    iov.iov_base = (void*)nlh;
    iov.iov_len = nlh->nlmsg_len;

    static struct msghdr outermsghdr;
    memset(&outermsghdr, 0, sizeof(struct msghdr));
    outermsghdr.msg_name = (void*)&dest_addr;
    outermsghdr.msg_namelen = sizeof(dest_addr);
    outermsghdr.msg_iov = &iov;
    outermsghdr.msg_iovlen = 1;

    int rc = sendmsg(sock_fd, &outermsghdr, 0);
    if(rc < 0){
        printf("message sending failed, error no: %d\n", errno);
    }
    return rc;
}