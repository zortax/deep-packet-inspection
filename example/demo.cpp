#include "dpi.h"
#include <iostream>
#include <cstdio>
#include <string>

using namespace std;

int main() {
    cout << "Connecting..." << endl;
    dpi_connect();

    if (dpi_state != DPI_Connected) {
        printf("Could not establish IPC connection. State: %d (Client state: "
               "%d)\n",
               dpi_state, client->state);
        return 2;
    }

    while (dpi_state == DPI_Connected) {
        p_buff *buf = pull_packet();
        if (!buf) {
            printf("Could not pull packet. State: %d (Client state: %d)\n",
                   dpi_state, client->state);
            return 0;
        }
        printf("Received packet. ID: %d  Length: %d\n", buf->packet_id,
               buf->len);
        push_packet(buf, DPI_ACCEPT);
    }

    return 0;
}
