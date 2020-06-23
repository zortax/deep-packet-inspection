// Copyright (C) 2020 Leonard Seibold
#ifdef __cplusplus
extern "C"{
#endif

#include <sys/socket.h>
#include <sys/un.h>

enum client_handler_state {
    Initialized = 1 << 0,
    Connected = 1 << 1,
    Error_SockCreate = 1 << 2,
    Error_Connect = 1 << 3,
    Error_Recv = 1 << 4,
    Error_Send = 1 << 5
};

typedef struct _client_handler client_handler;
struct _client_handler {
    enum client_handler_state state;

    int sock;
    struct sockaddr_un addr;
    struct msghdr msg;
    struct iovec iov;

    void (*connect)(client_handler *self);
    void (*destroy)(client_handler *self);
    int (*recv_msg)(client_handler *self, unsigned char *buf);
    int (*send_msg)(client_handler *self, unsigned char *buf, size_t len);
};

client_handler * create_client_handler(void);

void _client_handler_connect(client_handler *self);

void _client_handler_destroy(client_handler *self);

int _client_handler_recv_msg(client_handler *self, unsigned char *buf);

int _client_handler_send_msg(client_handler *self, unsigned char *buf, size_t len);

#ifdef __cplusplus
}
#endif

