// Copyright (C) 2020 Leonard Seibold
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/string.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#include "dpi_module.h"
#include "dpi_socketio.h"

MODULE_AUTHOR("Leonard Seibold <leo@zrtx.de>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LKM helping you to perform deep packet inspection");
MODULE_VERSION("0.1");

#define ANS_LEN     sizeof(unsigned int)

static sock_handler *sck_h;

static struct nf_hook_ops *nfho = NULL;

unsigned int hook_func(void *priv,
                       struct sk_buff *skb, 
                       const struct nf_hook_state *state) {
   
    unsigned char ans[ANS_LEN];

    if (sck_h->state != Connected) {
        return NF_ACCEPT;
    }

    sck_h->send_msg(sck_h, skb->data, skb->data_len);

    if (sck_h->state == Error_Send) {
        sck_h->disc_client(sck_h);
        sck_h->state = Initialized;
        // TODO: start kernelthread to accept
        return NF_ACCEPT;
    }

    sck_h->recv_msg(sck_h, ans, ANS_LEN);

    if (sck_h->state == Error_Recv) {
        sck_h->disc_client(sck_h);
        sck_h->state = Initialized;
        // TODO: start kernelthread to accept
        return NF_ACCEPT;
    }

    return (unsigned int) *ans;
}

static int __init LKM_init(void) {
    printk(KERN_INFO "Loading DPI Module...\n");

    sck_h = create_sock_handler();

    if (sck_h->state & (Error_SockCreate | Error_Bind)) {
        printk(KERN_ALERT "Couldn't create IPC socket. Exiting.");
        sck_h->destroy(sck_h);
        return -EIO;
    }

    nfho = (struct nf_hook_ops*) kcalloc(1, sizeof(struct nf_hook_ops), GFP_KERNEL);

    nfho->hook = (nf_hookfn*) hook_func;
    nfho->hooknum = NF_INET_PRE_ROUTING;
    nfho->pf = PF_INET;
    nfho->priority = NF_IP_PRI_FIRST;

    nf_register_net_hook(&init_net, nfho);

    sck_h->accept(sck_h);    

    return 0;
}

static void __exit LKM_exit(void) {
    printk(KERN_INFO "Cleaning up DPI Module...\n");
    sck_h->destroy(sck_h);    
    nf_unregister_net_hook(&init_net, nfho);
}

module_init(LKM_init);
module_exit(LKM_exit);

