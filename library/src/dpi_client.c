// Copyright (C) 2020 Leonard Seibold
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "dpi_shared_defs.h"
#include "dpi_client.h"

void _client_handler_connect(client_handler *self) {
    int len;

    if (self->state != Initialized) return;
    
    len = strlen(self->addr.sun_path) + sizeof(self->addr.sun_family);
    if (connect(self->sock, (struct sockaddr *) &(self->addr), len) < 0) {
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

int _client_handler_recv_msg(client_handler *self, unsigned char *buf) {
    int len;

    if (self->state != Connected) return 0;

    memset(&(self->msg), 0, sizeof(self->msg));
    memset(&(self->iov), 0, sizeof(self->iov));

    self->iov.iov_base = buf;
    self->iov.iov_len = ANS_SIZE;

    self->msg.msg_name = NULL;
    self->msg.msg_namelen = 0;
    self->msg.msg_iov = &(self->iov);
    self->msg.msg_iovlen = 1;
    self->msg.msg_control = NULL;
    self->msg.msg_controllen = 0;
    self->msg.msg_flags = 0;

    len = recvmsg(self->sock, &(self->msg), 0);

    if (len <= 0) {
        self->state = Error_Recv;
    }

    return len;
}

int _client_handler_send_msg(client_handler *self, unsigned char *buf, size_t len) {
    int sent;

    if (self->state != Connected) return 0;

    memset(&(self->msg), 0, sizeof(self->msg));
    memset(&(self->iov), 0, sizeof(self->iov));

    self->iov.iov_base = buf;
    self->iov.iov_len = len;

    self->msg.msg_name = NULL;
    self->msg.msg_name = 0;
    self->msg.msg_iov = &(self->iov);
    self->msg.msg_iovlen = 1;
    self->msg.msg_control = NULL;
    self->msg.msg_controllen = 0;
    self->msg.msg_flags = 0;

    sent = sendmsg(self->sock, &(self->msg), 0);

    if (sent < 0) {
        self->state = Error_Send;
    }

    return sent;
}

client_handler * create_client_handler(void) {
    client_handler *handler = (client_handler *) malloc(sizeof(client_handler));
   
    handler->connect = _client_handler_connect;
    handler->destroy = _client_handler_connect;
    handler->recv_msg = _client_handler_recv_msg;
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

