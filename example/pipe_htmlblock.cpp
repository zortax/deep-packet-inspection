// Copyright (C) 2020 Leonard Seibold
#include "dpi.h"
#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <stdlib.h>
#include <unistd.h>

int main() {

    p_buff packet;
    unsigned char data[MAX_BUF_SIZE];
    unsigned int verdict;

    while (1) {
        read_packet(STDIN_FILENO, &packet, data, &verdict);

        iphdr *ip = (iphdr *)packet.data;
        tcphdr *tcp = (tcphdr *)(((unsigned char *)&(ip->daddr)) + 4);

        if (ntohs(tcp->source) == 80)
            verdict = DPI_DROP;

        write_packet(STDOUT_FILENO, &packet, verdict);
    }

    return 0;
}
