// Copyright (C) 2020 Leonard Seibold 
#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

#include "dpi.h"

client_handler *client;
int dpi_state;
unsigned int (*callback_func)(p_buff *buf);

int dpi_connect(void) {
    client = create_client_handler();
    if (client->state != Initialized) {
        return 0;
    }
    client->connect(client);
    if (client->state != Connected)
        return 0;
    dpi_state |= DPI_Connected;
    return 1;
}

void dpi_set_callback(unsigned int (*callback)(p_buff *buf)) {
    callback_func = callback;
    dpi_state |= DPI_CallbackSet;
}

void start_callback_loop(void) {
    
    unsigned char buf[MAX_BUF_SIZE];
    size_t len;
    p_buff packet_buffer;
    unsigned int verdict;

    if (dpi_state == (DPI_Connected | DPI_CallbackSet)) {
       
        dpi_state |= DPI_Listening;

        while (1) {
            len = client->recv_msg(client, buf);
        
            if (len <= 0) {
                dpi_state |= DPI_Error_Recv;
                dpi_state ^= DPI_Listening;
                dpi_state ^= DPI_Connected;
                return;
            }

            memset(&packet_buffer, 0, sizeof(p_buff));
            packet_buffer.len = len;
            packet_buffer.data = buf;

            verdict = callback_func(&packet_buffer);
            len = client->send_msg(client, (unsigned char *) &verdict, ANS_SIZE);
            if (len < 0) {
                dpi_state |= DPI_Error_Send;
                dpi_state ^= DPI_Listening;
                dpi_state ^= DPI_Connected;
                return;
            }
        } 
    }
}

