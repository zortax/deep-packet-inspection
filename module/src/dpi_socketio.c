// Copyright (C) 2020 Leonard Seibold
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uio.h>
#include <asm/processor.h>

#include "dpi_socketio.h"

void _sock_handler_destroy(sock_handler *self) {
    if (!self) return;
    if (self->state == Connected && self->client) {
        self->client->ops->shutdown(self->client, SHUT_RDWR);
        kfree(self->client);
    }
    if ((self->state & (Initialized | Connected | Error_Send | Error_Recv)) && self->sock) {
        self->sock->ops->shutdown(self->sock, SHUT_RDWR);
        kfree(self->sock);
    }
    kfree(self);
}

void _sock_handler_accept(sock_handler *self) {
    int retval;

    if (self->state != Initialized) return;

    retval = self->sock->ops->listen(self->sock, LISTEN);
    if (retval == 0) {
        self->state = Error_Listen;
        return;
    }
    retval = self->sock->ops->accept(self->sock, self->client, 0, true);
    if (retval == 0) {
        self->state = Error_Accept;
        return;
    }
    self->state = Connected;
}

void _sock_handler_disc_client(sock_handler *self) {
    if (self->state & (Connected | Error_Send | Error_Recv)) {
        kfree(self->client);
        self->state = Initialized;
    }
}

void _sock_handler_send_msg(sock_handler *self, unsigned char *buf, size_t len) {
    mm_segment_t oldfs;
    int retval;

    if (self->state != Connected) return;

    memset(&(self->msg), 0, sizeof(self->msg));
    memset(&(self->iov), 0, sizeof(self->iov));

    self->iov.iov_base = buf;
    self->iov.iov_len = len;

    self->msg.msg_name = 0;
    self->msg.msg_namelen = 0;
    self->msg.msg_iter.type = ITER_IOVEC;
    self->msg.msg_iter.iov = &(self->iov);
    self->msg.msg_iter.count = 1;
    self->msg.msg_control = NULL;
    self->msg.msg_controllen = 0;
    self->msg.msg_flags = 0;

    oldfs = get_fs();
    set_fs(KERNEL_DS);

    retval = sock_sendmsg(self->client, &(self->msg));

    set_fs(oldfs);

    if (retval == 0) {
        self->state = Error_Send;
    }
}

void _sock_handler_recv_msg(sock_handler *self, unsigned char *buf, size_t len) {
    mm_segment_t oldfs;
    int retval;

    if (self->state != Connected) return;

    memset(&(self->msg), 0, sizeof(self->msg));
    memset(&(self->iov), 0, sizeof(self->iov));

    self->iov.iov_base = buf;
    self->iov.iov_len = len;

    self->msg.msg_name = 0;
    self->msg.msg_namelen = 0;
    self->msg.msg_iter.type = ITER_IOVEC;
    self->msg.msg_iter.iov = &(self->iov);
    self->msg.msg_iter.count = 1;
    self->msg.msg_control = NULL;
    self->msg.msg_controllen = 0;
    self->msg.msg_flags = 0;

    oldfs = get_fs();
    set_fs(KERNEL_DS);

    retval = sock_recvmsg(self->client, &(self->msg), len);

    set_fs(oldfs);

    if (retval == 0) {
        self->state = Error_Send;
    }
}

sock_handler * create_sock_handler(void) {
    
    int retval;

    sock_handler *handler = (sock_handler *) kcalloc(1, sizeof(sock_handler), GFP_KERNEL);

    handler->accept = _sock_handler_accept;
    handler->disc_client = _sock_handler_disc_client;
    handler->send_msg = _sock_handler_send_msg;
    handler->recv_msg = _sock_handler_recv_msg;

    retval = sock_create(AF_UNIX, SOCK_STREAM, 0, &(handler->sock));
    if (retval == 0) {
        handler->state = Error_SockCreate;
        return handler;
    }

    memset(&(handler->addr), 0, sizeof(handler->addr));
    handler->addr.sun_family = AF_UNIX;
    strcpy(handler->addr.sun_path, SOCKET_PATH);
    
    retval = handler->sock->ops->
        bind(handler->sock, (struct sockaddr *) &(handler->addr), sizeof(handler->addr));
    
    if (retval == 0) {
        handler->state = Error_Bind;
        return handler;
    }

    handler->state = Initialized;
    
    return handler;
}

