// Copyright (C) 2020 Leonard Seibold
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "dpi_socketio.h"

#define SZ_SOCKADDR     sizeof(struct sockaddr_un)
#define SZ_MSGHDR       sizeof(struct msghdr)

struct socket *sock = NULL;
struct socket *client = NULL;

struct msghdr *msg = NULL;
struct sockaddr_un *addr = NULL;

int setup_domain_socket(void) {
    int retval;

    addr = (struct sockaddr_un *) kcalloc(1, SZ_SOCKADDR, GFP_KERNEL);
    msg = (struct msghdr *) kcalloc(1, SZ_MSGHDR, GFP_KERNEL);

    retval = sock_create(AF_UNIX, SOCK_STREAM, 0, &sock);

    memset(addr, 0, SZ_SOCKADDR);
    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path, SOCKET_PATH);

    retval = sock->ops->bind(sock, addr, SZ_SOCKADDR);
    retval = sock->ops->listen(sock, LISTEN);
    retval = sock->ops->accept(sock, client, 0);

}

void send_message(unsigned char *buf, size_t len) {
    struct iovec iov;
    mm_segment_t oldfs;
    int retval;

    memset(msg, 0, SZ_MSGHDR);
    memset(&iov, 0, sizeof(iov));

    msg->msg_name = 0;
    msg->msg_namelen = 0;
    msg->msg_iov = &iov;
    msg->msg_iov->iob_base = buf;
    msg->msg_iov->iov_len = len+1;
    msg->msg_iovlen = 1;
    msg->msg_control = NULL;
    msg->msg_controllen = 0;
    msg->msg_flags = 0;

    oldfs = get_fs();
    set_fs(KERNEL_DS);
    retval = sock_sendmsg(client, &msg, len+1);
    set_fs(oldfs);
}

