// Copyright (C) 2020 Leonard Seibold
#include <asm/processor.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uio.h>
#include <net/sock.h>

#include "dpi_shared_defs.h"
#include "dpi_socketio.h"

void _sock_handler_destroy(sock_handler *self) {
    if (!self)
        return;
    if ((self->state & (Initialized | Connected | Error_Send | Error_Recv)) &&
        self->sock) {
        // kernel_sock_shutdown(self->sock, SHUT_RDWR);
        // sock_release(self->sock);
    }
    // kfree(self);
}

void _sock_handler_accept(sock_handler *self) {
    int retval;

    if (self->state != Initialized)
        return;

    retval = self->sock->ops->listen(self->sock, LISTEN);
    if (retval < 0) {
        printk(KERN_ALERT "Couldn't start listening.");
        self->state = Error_Listen;
        return;
    }

    self->client = (struct socket *)kmalloc(sizeof(struct socket), GFP_KERNEL);
    retval = sock_create(AF_UNIX, SOCK_STREAM, 0, &self->client);

    if (retval < 0) {
        printk(KERN_ALERT "Couldn't allocate/create client socket.");
        self->state = Error_Listen;
        return;
    }

    printk(KERN_INFO "Waiting for IPC client...");
    // retval = self->sock->ops->accept(self->sock, self->client, 0, true);
    retval = kernel_accept(self->sock, &(self->client), 0);
    if (retval < 0) {
        printk(KERN_ALERT "Couldn't accept IPC client.");
        self->state = Error_Accept;
        return;
    }
    printk(KERN_INFO "IPC client connected.");
    self->state = Connected;
}

void _sock_handler_disc_client(sock_handler *self) {
    if (self->state & (Connected | Error_Send | Error_Recv)) {
        kfree(self->client);
        self->state = Initialized;
    }
}

void _sock_handler_basic_send_msg(sock_handler *self, unsigned char *buf,
                                  size_t len) {
    int retval;
    int i;

    struct msghdr msg = {.msg_name = NULL,
                         .msg_namelen = 0,
                         .msg_control = NULL,
                         .msg_controllen = 0,
                         .msg_flags = 0};

    if (self->state != Connected)
        return;

    i = 0;
    while (i < len) {
        struct kvec iov = {
            .iov_base = (void *)((unsigned char *)buf + i),
            .iov_len = len - i,
        };
        retval = kernel_sendmsg(self->client, &msg, &iov, 1, iov.iov_len);
        if (retval < 0) {
            self->state = Error_Send;
            break;
        }
        i += retval;
    }
}

void _sock_handler_send_msg(sock_handler *self, unsigned char *buf,
                            size_t len) {

    printk(KERN_INFO "Sending data buffer with length %d", (int)len);
    // Send length of message
    self->basic_send_msg(self, (unsigned char *)&len, sizeof(size_t));
    // Send actual data
    self->basic_send_msg(self, buf, len);
}

int _sock_handler_basic_recv_msg(sock_handler *self, unsigned char *buf,
                                 size_t len) {
    struct kvec iov = {.iov_base = (void *)buf, .iov_len = len};

    struct msghdr msg = {.msg_name = NULL,
                         .msg_namelen = 0,
                         .msg_control = NULL,
                         .msg_controllen = 0,
                         .msg_flags = 0};

    return kernel_recvmsg(self->client, &msg, &iov, 1, len, msg.msg_flags);
}

int _sock_handler_recv_msg(sock_handler *self, unsigned char *buf) {
    int ret;
    size_t len;
    size_t received;

    ret = self->basic_recv_msg(self, (unsigned char *)&len, sizeof(size_t));
    
    printk(KERN_INFO " - Received data buffer length: %d. Bytes read: %d.", (int)len, ret);

    if (ret <= 0) {
        self->state = Error_Recv;
        return ret;
    }

    if (len > MAX_BUF_SIZE) {
        self->state = Error_Recv;
        return -1;
    }

    received = 0;
    while (received < len) {
        ret = self->basic_recv_msg(self, (unsigned char *)(buf + received),
                                   len - received);
        printk(KERN_INFO " - Received %d data buffer bytes.", ret);
        if (ret <= 0) {
            self->state = Error_Recv;
            break;
        }
        received += ret;
    }

    return received;
}

sock_handler *create_sock_handler(void) {

    int retval;

    sock_handler *handler =
        (sock_handler *)kcalloc(1, sizeof(sock_handler), GFP_KERNEL);

    handler->accept = _sock_handler_accept;
    handler->disc_client = _sock_handler_disc_client;
    handler->basic_send_msg = _sock_handler_basic_send_msg;
    handler->send_msg = _sock_handler_send_msg;
    handler->basic_recv_msg = _sock_handler_basic_recv_msg;
    handler->recv_msg = _sock_handler_recv_msg;

    retval = sock_create(AF_UNIX, SOCK_STREAM, 0, &(handler->sock));
    if (retval < 0) {
        handler->state = Error_SockCreate;
        return handler;
    }

    memset(&(handler->addr), 0, sizeof(handler->addr));
    handler->addr.sun_family = AF_UNIX;
    strcpy(handler->addr.sun_path, SOCK_PATH);

    retval = handler->sock->ops->bind(handler->sock,
                                      (struct sockaddr *)&(handler->addr),
                                      sizeof(struct msghdr));

    if (retval < 0) {
        handler->state = Error_Bind;
        return handler;
    }

    handler->state = Initialized;

    return handler;
}
