// Copyright (C) 2020 Leonard Seibold
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "dpi_module.h"
#include "dpi_worker.h"

MODULE_AUTHOR("Leonard Seibold <leo@zrtx.de>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LKM helping you to perform deep packet inspection");
MODULE_VERSION("0.9");

static int __init LKM_init(void) {
    printk(KERN_INFO "Loading DPI Module...\n");

    register_queue_handler();

    return launch_worker();
}

static void __exit LKM_exit(void) {
    printk(KERN_INFO "Cleaning up DPI Module...\n");
    unregister_queue_handler();
    stop_worker();
}

module_init(LKM_init);
module_exit(LKM_exit);
