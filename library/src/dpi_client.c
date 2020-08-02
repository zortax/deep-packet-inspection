// Copyright (C) 2020 Leonard Seibold
#include "dpi_client.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "dpi_shared_defs.h"

void _client_handler_connect(client_handler *self) {
    int len;

    if (self->state != Initialized)
        return;

    len = strlen(self->addr.sun_path) + sizeof(self->addr.sun_family);
    if (connect(self->sock, (struct sockaddr *)&(self->addr), len) < 0) {
        self->state = Error_Connect;
    } else {
        self->state = Connected;
    }
}

void _client_handler_destroy(client_handler *self) {
    if (self->state == Connected) {
        close(self->sock);
    }
    free(self);
}

int _client_handler_basic_recv_msg(client_handler *self, unsigned char *buf,
                                   size_t len) {
    struct iovec iov = {.iov_base = (void *)buf, .iov_len = len};

    struct msghdr msg = {.msg_name = NULL,
                         .msg_namelen = 0,
                         .msg_iov = &iov,
                         .msg_iovlen = 1,
                         .msg_control = NULL,
                         .msg_controllen = 0,
                         .msg_flags = 0};

    int ret = recvmsg(self->sock, &msg, 0);

    if (ret <= 0)
        self->state = Error_Recv;

    return ret;
}

int _client_handler_recv_msg(client_handler *self, unsigned char **buf,
                             int allocate) {
    int ret;
    size_t len;
    size_t received;

    ret = self->basic_recv_msg(self, (unsigned char *)&len, sizeof(size_t));

    if (ret <= 0) {
        D(printf("Failed to receive data buffer length. Return value: %d\n",
                 ret));
        return ret;
    }

    if (len > MAX_BUF_SIZE) {
        self->state = Error_Recv;
        D(printf(
            "Received data buffer length (%d) is bigger than maximum buffer "
            "size (%d)\n",
            (int)len, MAX_BUF_SIZE));
        return -1;
    }

    if (allocate)
        *buf = (unsigned char *)malloc(len);

    D(printf("Received data buffer length: %d\n", (int)len));
    received = 0;
    while (received < len) {
        ret = self->basic_recv_msg(self, (unsigned char *)((*buf) + received),
                                   len - received);
        if (ret <= 0) {
            D(printf("Failed to receive data after receiving %d bytes.\n",
                     (int)received));
            self->state = Error_Recv;
            if (allocate)
                free(*buf);
            break;
        }
        received += ret;
    }

    return received;
}

int _client_handler_basic_send_msg(client_handler *self, unsigned char *buf,
                                   size_t len) {
    int retval;
    size_t i;

    struct msghdr msg = {.msg_name = NULL,
                         .msg_namelen = 0,
                         .msg_iovlen = 1,
                         .msg_controllen = 0,
                         .msg_flags = 0};

    if (self->state != Connected)
        return -1;

    i = 0;
    while (i < len) {
        struct iovec iov = {.iov_base = (void *)((unsigned char *)buf + i),
                            .iov_len = len - i};
        msg.msg_iov = &iov;
        retval = sendmsg(self->sock, &msg, msg.msg_flags);
        if (retval < 0) {
            self->state = Error_Send;
            D(printf("Error while send: %d\n", errno));
            break;
        }
        i += retval;
    }

    return i;
}

int _client_handler_send_msg(client_handler *self, unsigned char *buf,
                             size_t len) {
    int ret;

    ret = self->basic_send_msg(self, (unsigned char *)&len, sizeof(size_t));
    if (ret < 0) {
        D(printf("Sent less than 0 bytes.\n"));
        return -1;
    }
    ret = self->basic_send_msg(self, buf, len);
    D(printf("Sent data buffer. Return value: %d.\n", ret));
    return ret;
}

client_handler *create_client_handler(void) {
    client_handler *handler = (client_handler *)malloc(sizeof(client_handler));

    handler->connect = _client_handler_connect;
    handler->destroy = _client_handler_destroy;
    handler->basic_recv_msg = _client_handler_basic_recv_msg;
    handler->recv_msg = _client_handler_recv_msg;
    handler->basic_send_msg = _client_handler_basic_send_msg;
    handler->send_msg = _client_handler_send_msg;

    handler->sock = socket(AF_UNIX, SOCK_STREAM, 0);

    if (handler->sock < 0) {
        handler->state = Error_SockCreate;
        return handler;
    }

    memset(&(handler->addr), 0, sizeof(handler->addr));

    handler->addr.sun_family = AF_UNIX;
    strcpy(handler->addr.sun_path, SOCK_PATH);

    handler->state = Initialized;
    return handler;
}
