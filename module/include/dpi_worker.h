// Copyright (C) 2020 Leonard Seibold
#include "dpi_netfilter.h"
#include "dpi_shared_defs.h"
#include "dpi_socketio.h"

#include <linux/sched/signal.h>

extern sock_handler *sck_h;

int launch_worker(void);

void stop_worker(void);
