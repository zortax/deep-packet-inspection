#include "dpi.h"
#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <stdio.h>
#include <unistd.h>

int main() {

    p_buff packet;
    unsigned char data[MAX_BUF_SIZE];
    unsigned int verdict;

    while (1) {
        read_packet(STDIN_FILENO, &packet, data, &verdict);

        iphdr *ip = (iphdr *)packet.data;
        // TCP Header starts 4 Bytes after ip->daddr (as long as IHL is not > 5)
        tcphdr *tcp = (tcphdr *)(((unsigned char *)&(ip->daddr)) + 4);

        struct in_addr src {
            .s_addr = (unsigned int)ip->saddr
        };
        struct in_addr dest {
            .s_addr = (unsigned int)ip->daddr
        };

        fprintf(stderr,
                "Read packet. Length: %d  Source IP: %s  Dest IP: %s  Source "
                "port: %d  "
                "Dest port: %d\n",
                packet.len, inet_ntoa(src), inet_ntoa(dest), ntohs(tcp->source),
                ntohs(tcp->dest));

        write_packet(STDOUT_FILENO, &packet, DPI_ACCEPT);
    }

    return 0;
}
