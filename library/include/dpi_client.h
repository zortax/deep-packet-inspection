// Copyright (C) 2020 Leonard Seibold
#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>
#include <sys/un.h>

enum client_handler_state {
    Initialized = 1 << 0,      // 1
    Connected = 1 << 1,        // 2
    Error_SockCreate = 1 << 2, // 4
    Error_Connect = 1 << 3,    // 8
    Error_Recv = 1 << 4,       // 16
    Error_Send = 1 << 5        // 32
};

typedef struct _client_handler client_handler;
struct _client_handler {
    enum client_handler_state state;

    int sock;
    struct sockaddr_un addr;

    void (*connect)(client_handler *self);
    void (*destroy)(client_handler *self);
    int (*basic_recv_msg)(client_handler *self, unsigned char *buf, size_t len);
    int (*recv_msg)(client_handler *self, unsigned char *buf, int allocate);
    int (*basic_send_msg)(client_handler *self, unsigned char *buf, size_t len);
    int (*send_msg)(client_handler *self, unsigned char *buf, size_t len);
};

client_handler *create_client_handler(void);

void _client_handler_connect(client_handler *self);

void _client_handler_destroy(client_handler *self);

int _client_handler_recv_msg(client_handler *self, unsigned char *buf,
                             int allocate);

int _client_handler_send_msg(client_handler *self, unsigned char *buf,
                             size_t len);

int _client_handler_basic_recv_msg(client_handler *self, unsigned char *buf,
                                   size_t len);

int _client_handler_basic_send_msg(client_handler *self, unsigned char *buf,
                                   size_t len);

#ifdef __cplusplus
}
#endif
