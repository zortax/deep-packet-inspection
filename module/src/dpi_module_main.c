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

static sock_handler *sck_h;

static struct nf_hook_ops *nfho = NULL;

unsigned int hook_func(void *priv,
                       struct sk_buff *skb, 
                       const struct nf_hook_state *state) {
    
    struct iphdr *iph;
    struct tcphdr *tcph;

    iph = ip_hdr(skb);


    if (iph->protocol == IPPROTO_TCP) {
        // Packet is TCP packet
        tcph = tcp_hdr(skb);

        // HTTPS
        if (ntohs(tcph->dest) == 80) {
           printk(KERN_INFO "HTTP Packet..!");
           memcpy(show_buf, (char *) skb->data, skb->data_len);
        }

    }

    return NF_ACCEPT;
}

static int __init LKM_init(void) {
    printk(KERN_INFO "Loading DPI Module...\n");

    sck_h = create_sock_handler();

    param_buf = kzalloc(PAGE_SIZE, GFP_KERNEL);
    show_buf = kzalloc(PAGE_SIZE, GFP_KERNEL);
    register_kobj = kobject_create_and_add("dpi", kernel_kobj);
    if (!register_kobj)
        return -ENOMEM;

    if (sysfs_create_group(register_kobj, &reg_attr_group)) {
        kobject_put(register_kobj);
        return -ENOMEM;
    }

    nfho = (struct nf_hook_ops*) kcalloc(1, sizeof(struct nf_hook_ops), GFP_KERNEL);

    nfho->hook = (nf_hookfn*) hook_func;
    nfho->hooknum = NF_INET_PRE_ROUTING;
    nfho->pf = PF_INET;
    nfho->priority = NF_IP_PRI_FIRST;

    nf_register_net_hook(&init_net, nfho);

    sprintf(show_buf, "Hello World!");

    sck_h->accept(sck_h);    

    return 0;
}

static void __exit LKM_exit(void) {
    printk(KERN_INFO "Cleaning up DPI Module...\n");
    
    sck_h->destroy(sck_h);    

    kfree(param_buf);
    kfree(show_buf);
    kobject_put(register_kobj);
    nf_unregister_net_hook(&init_net, nfho);
}

module_init(LKM_init);
module_exit(LKM_exit);

