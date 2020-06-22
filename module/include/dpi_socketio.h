// Copyright (C) 2020 Leonard Seibold
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/un.h>
#include <net/sock.h>
#include <uapi/linux/un.h>

#define SOCKET_PATH         "/var/packetstream.socket"
#define LISTEN              10

enum sock_handler_state {
    Initialized = 1 << 0,
    Connected = 1 << 1,
    Error_SockCreate = 1 << 2,
    Error_Bind = 1 << 3,
    Error_Listen = 1 << 4,
    Error_Accept = 1 << 5,
    Error_Send = 1 << 6,
    Error_Recv = 1 << 7
};

typedef struct _sock_handler sock_handler;
struct _sock_handler {
    enum sock_handler_state state;

    struct socket *sock;
    struct socket *client;
    struct msghdr msg;
    struct sockaddr_un addr;
    struct iovec iov;

    void (*destroy)(sock_handler *self);
    void (*accept)(sock_handler *self);
    void (*disc_client)(sock_handler *self);
    void (*send_msg)(sock_handler *self, unsigned char *buf, size_t len);
    void (*recv_msg)(sock_handler *self, unsigned char *buf, size_t len);
};

sock_handler * create_sock_handler(void);

void _sock_handler_destroy(sock_handler *self);

void _sock_handler_accept(sock_handler *self);

void _sock_handler_disc_client(sock_handler *self);

void _sock_handler_send_msg(sock_handler *self, unsigned char *buf, size_t len);

void _sock_handler_recv_msg(sock_handler *self, unsigned char *buf, size_t len);

