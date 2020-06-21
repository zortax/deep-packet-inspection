// Copyright (C) 2020 Leonard Seibold
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/un.h>
#include <net/socket.h>

#define SOCKET_PATH         "/var/packetstream.socket"
#define LISTEN              10
#define MAX_ANSWER_SIZE     8

int setup_domain_socket(void);

void send_message(unsigned char *buf);

void recv_message(unsigned char *buf);

