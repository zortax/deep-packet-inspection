// Copyright (C) 2020 Leonard Seibold
#define _GNU_SOURCE

#include <stdio.h>
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

p_buff *pull_packet(void) {
    unsigned char *data_buf = NULL;
    p_buff *packet_buffer = NULL;
    size_t read;

    if (dpi_state != DPI_Connected)
        return NULL;

    packet_buffer = (p_buff *)malloc(sizeof(p_buff));

    read = client->basic_recv_msg(client,
                                  (unsigned char *)&(packet_buffer->packet_id),
                                  sizeof(packet_buffer->packet_id));

    if (read <= 0) {
        dpi_state |= DPI_Error_Recv;
        printf("Couldn't read packet id. Return value: %d\n", (int)read);
        return NULL;
    }

    read = client->recv_msg(client, data_buf, 1);

    if (read <= 0) {
        dpi_state |= DPI_Error_Recv;
        printf("Couldn't read data buffer. Return value: %d\n", (int)read);
        return NULL;
    }

    packet_buffer->len = read;
    packet_buffer->data = data_buf;

    return packet_buffer;
}

void push_packet(p_buff *buf, unsigned int verdict) {
    size_t sent;

    if (dpi_state != DPI_Connected)
        goto clean;

    sent = client->basic_send_msg(client, (unsigned char *)&(buf->packet_id),
                                  sizeof(buf->packet_id));

    if (sent < sizeof(buf->packet_id)) {
        dpi_state = DPI_Error_Send;
        goto clean;
    }

    sent = client->basic_send_msg(client, (unsigned char *)&verdict,
                                  sizeof(verdict));

    if (sent < sizeof(buf->packet_id)) {
        dpi_state = DPI_Error_Send;
        goto clean;
    }

    sent = client->send_msg(client, buf->data, buf->len);

    if (sent < buf->len) {
        dpi_state = DPI_Error_Send;
        goto clean;
    }

clean:
    free(buf->data);
    free(buf);
}
