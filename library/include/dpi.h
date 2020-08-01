// Copyright (C) 2020 Leonard Seibold
#ifdef __cplusplus
extern "C" {
#endif

#include "dpi_client.h"
#include "dpi_shared_defs.h"

#define DPI_DROP 0
#define DPI_ACCEPT 1

enum client_state {
    DPI_Connected = 1 << 0,  // 1
    DPI_Error_Recv = 1 << 1, // 2
    DPI_Error_Send = 1 << 2  // 4
};

typedef struct p_buff {
    int packet_id;
    size_t len;
    unsigned char *data;
} p_buff;

extern client_handler *client;
extern int dpi_state;

int dpi_connect(void);
p_buff *pull_packet(void);
void push_packet(p_buff *buf, unsigned int verdict);

#ifdef __cplusplus
}
#endif
