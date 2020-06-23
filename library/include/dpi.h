// Copyright (C) 2020 Leonard Seibold 
#ifdef __cplusplus
extern "C"{
#endif

#include "dpi_shared_defs.h"
#include "dpi_client.h"

#define DPI_DROP    0
#define DPI_ACCEPT  1

extern client_handler *client;

typedef struct _p_buff p_buff;
struct p_buff {
    unsigned int len;
    unsigned char *data;
};

int dpi_connect();
int dpi_set_callback(unsigned int (*callback)(p_buff *buf));

#ifdef __cplusplus
}
#endif
