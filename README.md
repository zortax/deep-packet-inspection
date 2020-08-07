# deep-packet-inspection
A Loadable Kernel Module for Deep Packet Inspection using the netfilter API.

This Module provides a simple NF_QUEUE backend (similiar but much simpler than
the existing  [nfnetlink](https://www.netfilter.org/projects/libnfnetlink/) 
subsystem) as well as a userland library to help you inspect, filter and 
minupulate packets in userspace.

## Usage

A minimal userspace packet filter (that ignores all errors that may ocurr) to 
drop all packets could look like this:
```C
#include <stdlib.h>
#include "dpi.h"

int main() {
  
  dpi_connect();
  
  while (1) {
    p_buff *packet = pull_packet();
    push_packet(packet, DPI_DROP);
    free(packet->data);
    free(packet);
  }

  return 0;
}
```

More sophisticated examples can be found in the [example directory](example).

**Disclaimer**:  This project is highly experimental and can in no way be 
considered *stable*, the kernel module may panic your kernel at any time.

## Building
To build the kernel module and the library, simply run the `build.sh` script:
```
$ ./build.sh
```
Make sure you have the [linux headers as well as KBuild](https://www.archlinux.org/packages/core/x86_64/linux-headers/)
installed.

The kernel module can then be loaded and unloaded with the following scripts:
```
root# ./load_module.sh
root# ./unload_module.sh
```
After unloading the module, you should delete `/var/packetstream.sock`, otherwise
kernel module will not load again.

The examples can be build with
```
$ ./build_demos.sh
```
and then be executed by executing
```
root# ./launch_demo.sh <demo_name>
```
