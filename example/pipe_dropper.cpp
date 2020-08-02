#include "dpi.h"
#include <unistd.h>

int main() {

    p_buff packet;
    unsigned char data[MAX_BUF_SIZE];
    unsigned int verdict;

    while (1) {
        read_packet(STDIN_FILENO, &packet, data, &verdict);
        write_packet(STDOUT_FILENO, &packet, DPI_DROP);
    }

    return 0;
}
