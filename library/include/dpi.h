// Copyright (C) 2020 Leonard Seibold 
#ifdef __cplusplus
extern "C"{
#endif

#include "dpi_shared_defs.h"
#include "dpi_client.h"

#define DPI_DROP    0
#define DPI_ACCEPT  1

extern client_handler *client;

enum client_state {
    DPI_Connected = 1 << 0,
    DPI_CallbackSet = 1 << 1,
    DPI_Listening = 1 << 2,
    DPI_Error_Recv = 1 << 3,
    DPI_Error_Send = 1 << 4
};

extern int dpi_state;

typedef struct p_buff {
    unsigned int len;
    unsigned char *data;
} p_buff;

extern unsigned int (*callback_func)(p_buff *buf);

int dpi_connect(void);
void dpi_set_callback(unsigned int (*callback)(p_buff *buf));
void start_callback_loop(void);

#ifdef __cplusplus
}
#endif
