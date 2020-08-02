// Copyright (C) 2020 Leonard Seibold
#include "dpi_netfilter.h"
#include <net/net_namespace.h>

struct task_struct *worker = NULL;

DEFINE_MUTEX(sleep_lock);

int id_stack[QUEUE_CAPACITY];
size_t id_stack_back;
DEFINE_MUTEX(id_stack_lock);

struct nf_queue_entry *queued_packets[QUEUE_CAPACITY];

inline static bool id_available(void) { return id_stack_back > 0; }

static int pop_id(void) {
    int val;
    if (id_stack_back <= 0)
        val = -1;
    val = id_stack[--id_stack_back];
    return val;
}

void push_id(int id) {
    if (id_stack_back < QUEUE_CAPACITY)
        id_stack[id_stack_back++] = id;
}

int send_queue[QUEUE_CAPACITY];
size_t sq_front = 0;
static size_t sq_back = 0;
size_t sq_cur_size = 0;
DEFINE_MUTEX(sq_lock);

bool send_queue_empty(void) { return sq_cur_size == 0; }

static void push_back_packet(int id) {
    if (sq_cur_size < QUEUE_CAPACITY) {
        if (sq_back < QUEUE_CAPACITY)
            send_queue[sq_back++] = id;
        else {
            sq_back -= QUEUE_CAPACITY;
            send_queue[sq_back++] = id;
        }
        sq_cur_size++;
    }
}

int pop_front_packet(void) {
    int val;
    if (sq_cur_size > 0) {
        if (sq_front < QUEUE_CAPACITY)
            val = send_queue[sq_front++];
        else {
            sq_front -= QUEUE_CAPACITY;
            val = send_queue[sq_front++];
        }
        sq_cur_size--;
    } else
        val = -1;
    return val;
}

static int dpi_outfn(struct nf_queue_entry *entry, unsigned int queuenum) {
    int id;

    printk(KERN_INFO "RECEIVED QUEUED PACKET!!!");

    if (!worker)
        return -1;

    printk(KERN_INFO "Locking mutexes..");
    mutex_lock(&sq_lock);
    mutex_lock(&id_stack_lock);

    if (!id_available() || (sq_cur_size >= QUEUE_CAPACITY)) {
        printk(KERN_INFO "No ID available. Exiting.");
        mutex_unlock(&sq_lock);
        mutex_unlock(&id_stack_lock);

        return -1;
    }

    id = pop_id();

    printk(KERN_INFO "Popped packet id: %d", id);

    mutex_unlock(&id_stack_lock);

    queued_packets[id] = entry;

    push_back_packet(id);
    printk(KERN_INFO "Pushed packet id to send queue.");

    mutex_unlock(&sq_lock);

    if (!mutex_trylock(&sleep_lock)) {
        printk(KERN_INFO "Waking up worker thread.");
        wake_up_process(worker);
    } else {
        mutex_unlock(&sleep_lock);
        printk(KERN_INFO "Worker thread alread alive.");
    }
    return 0;
}

static void dpi_nf_hook_drop(struct net *net) {
    // Nothing for now
}

static struct nf_queue_handler qh = {
    .outfn = dpi_outfn,
    .nf_hook_drop = dpi_nf_hook_drop,
};

static int __net_init dpi_net_init(struct net *net) {
    printk(KERN_INFO "Registering queue handler for a net.");
    nf_register_queue_handler(net, &qh);
    return 0;
}

static void __net_exit dpi_net_exit(struct net *net) {
    nf_unregister_queue_handler(net);
}

static void dpi_net_exit_batch(struct list_head *net_exit_list) {
    // I don't care :–)
    // Doesn't seem to happen... ¯\_(ツ)_/¯
    synchronize_rcu();
}

static struct pernet_operations dpi_net_ops = {
    .init = dpi_net_init,
    .exit = dpi_net_exit,
    .exit_batch = dpi_net_exit_batch,
};

void register_queue_handler(void) {
    int ret;
    int i;

    // populate ID queue
    i = QUEUE_CAPACITY;
    while (i-- > 0)
        id_stack[i] = i;
    id_stack_back = QUEUE_CAPACITY;

    ret = register_pernet_subsys(&dpi_net_ops);
    printk(KERN_INFO "Tried registering queue handler. Return value: %d", ret);
}

void unregister_queue_handler(void) { unregister_pernet_subsys(&dpi_net_ops); }
