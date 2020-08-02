// Copyright (C) 2020 Leonard Seibold
#include "dpi_worker.h"

sock_handler *sck_h = NULL;

static struct task_struct *recv_worker = NULL;

static void shutdown_client(void) {
    kernel_sock_shutdown(sck_h->client, SHUT_RDWR);
    sock_release(sck_h->client);
}

static int recv_worker_main(void *arg) {
    unsigned char ans[MAX_BUF_SIZE];
    int read = 0;
    int id;
    unsigned int verdict;

    allow_signal(SIGKILL);
    allow_signal(SIGTERM);

    while (!kthread_should_stop()) {
        struct nf_queue_entry *entry;
        D(printk(KERN_INFO "Waiting for answer from IPC client..."));
        read = sck_h->basic_recv_msg(sck_h, (unsigned char *)&id, sizeof(int));

        D(printk(KERN_INFO " - Received packet ID: %d. Bytes read: %d", (int)id,
               read));
        if (read <= 0)
            return 0;

        read = sck_h->basic_recv_msg(sck_h, (unsigned char *)&verdict,
                                     sizeof(unsigned int));

        D(printk(KERN_INFO " - Received verdict: %d. Bytes read: %d",
               (int)verdict, read));
        if (read <= 0)
            return 0;

        read = sck_h->recv_msg(sck_h, ans);
        D(printk(KERN_INFO " - Received data buffer. Bytes read: %d", read));
        if (read <= 0)
            return 0;

        entry = queued_packets[id];

        if (verdict == NF_ACCEPT)
            memcpy((void *)entry->skb->data, (const void *)ans,
                   entry->skb->data_len);

        printk(KERN_INFO "Reinjecting packet. ID: %d Verdict: %d", id, verdict);

        nf_reinject(entry, verdict);

        mutex_lock(&id_stack_lock);
        push_id(id);
        mutex_unlock(&id_stack_lock);
    }

    return 0;
}

static int worker_main(void *arg) {
    // char *msg = "Hello World!";

    allow_signal(SIGKILL);
    allow_signal(SIGTERM);

    sck_h = create_sock_handler();

    if (sck_h->state & (Error_SockCreate | Error_Bind)) {
        printk(KERN_ALERT "Couldn't create IPC socket (Error: %d). Exiting.",
               sck_h->state);
        // TODO: memory leak :–(
        // sck_h->destroy(sck_h);
        return -EIO;
    }

    while (!kthread_should_stop()) {
        // For now we will only accept one client so we do not have to worry
        // about synchronization and to wich client packets should be send
        sck_h->accept(sck_h);

        // sck_h->send_msg(sck_h, (unsigned char *)msg, 12);

        recv_worker = kthread_run(recv_worker_main, NULL, KBUILD_MODNAME);

        if (IS_ERR(recv_worker)) {
            // Oh no :(
            shutdown_client();
            printk(KERN_ALERT "Could start kernel thread to receive verdicts.");
            return -1;
        }

        while (!kthread_should_stop()) {
            int id;
            struct nf_queue_entry *entry;
            // prevent lost wake-up
            set_current_state(TASK_INTERRUPTIBLE);
            mutex_lock(&sq_lock);
            if (send_queue_empty()) {
                mutex_unlock(&sq_lock);
                mutex_lock(&sleep_lock);
                D(printk(KERN_INFO "Send queue is empty. Going to sleep..."));
                schedule();
                mutex_unlock(&sleep_lock);
                mutex_lock(&sq_lock);
                D(printk(KERN_INFO "Woke up."));
            }
            set_current_state(TASK_RUNNING);

            id = pop_front_packet();
            mutex_unlock(&sq_lock);

            printk(KERN_INFO "Sending packet with ID %d.", id);

            entry = queued_packets[id];

            sck_h->basic_send_msg(sck_h, (unsigned char *)&id, sizeof(int));
            sck_h->send_msg(sck_h, entry->skb->data,
                            (size_t)ntohs(ip_hdr(entry->skb)->tot_len));

            printk(KERN_INFO "Sent packet.");

            if (sck_h->state & Error_Send) {
                // Client disconnected
                D(printk(KERN_INFO "Client disconnected (?)"));
                break;
            }
        }

        shutdown_client();
    }
    return 0;
}

int launch_worker(void) {

    worker = kthread_run(worker_main, NULL, KBUILD_MODNAME);

    if (IS_ERR(worker)) {
        printk(KERN_ALERT "Couldn't create kernel thread for worker. Exiting.");
        return -1;
    }

    return 0;
}

void stop_worker(void) {
    send_sig(SIGTERM, worker, 1);
    kthread_stop(worker);
    /*if (recv_worker) {
        send_sig(SIGTERM, recv_worker, 1);
        kthread_stop(recv_worker);
    }*/ // Yeah whatever just leak it better than constantly panicking the VM
}
