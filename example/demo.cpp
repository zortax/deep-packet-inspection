#include <iostream>
#include "dpi.h"

using namespace std;

unsigned int hook_func(p_buff *buf) {

    return DPI_DROP;
}

int main() {
    cout << "Connecting..." << endl;
    dpi_connect();
    cout << "Setting callback..." << endl;
    dpi_set_callback(hook_func); 
    cout << "Starting loop..." << endl;
    start_callback_loop();
    cout << "Loop stopped. State: " << dpi_state << " (handler state: " << client->state << ")" << endl;
    return 0;
}
