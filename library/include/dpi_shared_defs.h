// Copyright (C) 2020 Leonard Seibold
#ifdef __cplusplus
extern "C" {
#endif

#ifdef RELEASE
#define D(x)                                                                   \
    do {                                                                       \
    } while (0)
#else
#define D(x) x
#endif

#define MODULE_NAME "dpi"
#define SOCK_PATH "/var/packetstream.sock"
#define MAX_BUF_SIZE                                                           \
    2000 /* Amount of memory allocated for packet buffer.                      \
            Rationale: By defining a maximum buffer size,                      \
            we can statically allocate the buffer on the                       \
            stack and don't need to allocate it for every                      \
            answer which would be slow (or deal with                           \
            reallocation). This should not be a problem                        \
            as MTU usually doesn't exceed 1500 Bytes. */

#ifdef __cplusplus
}
#endif
