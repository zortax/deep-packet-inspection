// Copyright (C) 2020 Leonard Seibold
#include <asm/string.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/preempt.h>
#include <linux/slab.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#include "dpi_module.h"
#include "dpi_worker.h"

MODULE_AUTHOR("Leonard Seibold <leo@zrtx.de>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LKM helping you to perform deep packet inspection");
MODULE_VERSION("0.1");

static struct nf_hook_ops *nfho = NULL;

unsigned int hook_func(void *priv, struct sk_buff *skb,
                       const struct nf_hook_state *state) {

    struct iphdr *iph;
    struct tcphdr *tcph;
    iph = ip_hdr(skb);

    if (!sck_h)
        return NF_ACCEPT;

    if (iph->protocol != IPPROTO_TCP)
        return NF_ACCEPT;

    tcph = tcp_hdr(skb);

    if (!(ntohs(tcph->dest) == 80 || ntohs(tcph->source) == 80))
        return NF_ACCEPT;

    if (sck_h->state != Connected) {
        return NF_ACCEPT;
    }

    return NF_QUEUE;
}

static int __init LKM_init(void) {
    printk(KERN_INFO "Loading DPI Module...\n");

    nfho = (struct nf_hook_ops *)kcalloc(1, sizeof(struct nf_hook_ops),
                                         GFP_KERNEL);

    nfho->hook = (nf_hookfn *)hook_func;
    nfho->hooknum = NF_INET_PRE_ROUTING;
    nfho->pf = PF_INET;
    nfho->priority = NF_IP_PRI_FIRST;

    nf_register_net_hook(&init_net, nfho);

    register_queue_handler();

    return launch_worker();
}

static void __exit LKM_exit(void) {
    printk(KERN_INFO "Cleaning up DPI Module...\n");
    nf_unregister_net_hook(&init_net, nfho);
    kfree(nfho);
    unregister_queue_handler();
    stop_worker();
}

module_init(LKM_init);
module_exit(LKM_exit);
