// Copyright (C) 2020 Leonard Seibold
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <net/netfilter/nf_queue.h>

#define QUEUE_CAPACITY 128

extern struct task_struct *worker;

extern struct mutex sleep_lock;

extern int id_stack[QUEUE_CAPACITY];
extern size_t id_stack_back;
extern struct mutex id_stack_lock;

extern struct nf_queue_entry *queued_packets[QUEUE_CAPACITY];

void push_id(int id);

extern int send_queue[QUEUE_CAPACITY];
extern size_t sq_front;
extern size_t sq_cur_size;
extern struct mutex sq_lock;

bool send_queue_empty(void);

int pop_front_packet(void);

void register_queue_handler(void);

void unregister_queue_handler(void);
