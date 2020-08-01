// Copyright (C) 2020 Leonard Seibold
#define MODULE_NAME "dpi"
#include <linux/netfilter.h>

unsigned int hook_func(void *priv, struct sk_buff *skb,
                       const struct nf_hook_state *state);
