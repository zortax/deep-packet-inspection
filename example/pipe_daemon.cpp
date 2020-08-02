#include <cstdio>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "dpi.h"

#define OUT_PIPE "/tmp/packetout"
#define IN_PIPE "/tmp/packetin"

#define LOG_PREFIX "[Preifendämon 😈] "
#define log(format, ...)                                                       \
    printf(string(LOG_PREFIX)                                                  \
               .append(format)                                                 \
               .append("\n")                                                   \
               .c_str() __VA_OPT__(, ) __VA_ARGS__)

using namespace std;

const unsigned int default_verdict = DPI_ACCEPT;

void write_packets() {

    int fd = open(OUT_PIPE, O_WRONLY);

    if (fd < 0) {
        log("Couldn't open output FIFO.");
        exit(EXIT_FAILURE);
    }

    while (1) {
        p_buff *packet = pull_packet();
        if (!packet) {
            log("Failed to pull packet. Is the LKM still active?");
            exit(EXIT_FAILURE);
        }
        log("Pulled packet: ID=%d, Length=%d.", packet->packet_id,
            (int)packet->len);
        write(fd, &(packet->packet_id), sizeof(packet->packet_id));
        write(fd, &default_verdict, sizeof(default_verdict));
        write(fd, &(packet->len), sizeof(packet->len));
        write(fd, packet->data, packet->len);
        delete packet;
    }
}

void read_packets() {

    int fd = open(IN_PIPE, O_RDONLY);

    if (fd < 0) {
        log("Couldn't open input FIFO.");
        exit(EXIT_FAILURE);
    }

    while (1) {
        p_buff packet(false);
        unsigned int verdict;
        unsigned char data[MAX_BUF_SIZE];

        read(fd, &(packet.packet_id), sizeof(packet.packet_id));
        read(fd, &verdict, sizeof(verdict));
        read(fd, &(packet.len), sizeof(packet.len));
        read(fd, data, packet.len);

        packet.data = data;

        push_packet(&packet, verdict);
        if (dpi_state != DPI_Connected) {
            log("Failed to push packet back to LKM (State=%d, ClientState=%d).",
                dpi_state, client->state);
        }
        log("Pushed packet back to LKM: ID=%d, Length=%d, Verdict=%d.",
            packet.packet_id, (int)packet.len, verdict);
    }
}

int main(int argc, char **argv) {

    size_t pid;

    char out_fifo[sizeof(OUT_PIPE)];
    char in_fifo[sizeof(IN_PIPE)];

    strcpy(out_fifo, OUT_PIPE);
    strcpy(in_fifo, IN_PIPE);

    mkfifo(out_fifo, 0666);
    mkfifo(in_fifo, 0666);

    log("Created FIFOs.");

    log("Trying to connect to DPI LKM...");
    if (!dpi_connect()) {
        log("Couldn't connect to LKM 😢. Client state is %d.", client->state);
        return EXIT_FAILURE;
    }

    log("Connected to DPI LKM.");

    pid = fork();

    if (pid == 0) {
        write_packets();
    } else if (pid > 0) {
        read_packets();
    } else {
        perror("Fork failed.");
        return EXIT_FAILURE;
    }
}
