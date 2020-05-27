#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define MODULE_NAME "dpi"

MODULE_AUTHOR("Leonard Seibold <leo@zrtx.de>");
MODULE_LICENSE("MIT");
MODULE_DESCRIPTION("LKM helping you to perform deep packet inspection");
MODULE_VERSION("0.1");

static int __init LKM_init(void) {

    printk(KERN_INFO "Loading DPI Module...\n");
    return 0;
}

static void __exit LKM_exit(void) {
    
    printk(KERN_INFO "Cleaning up DPI Module...\n");
    
}

module_init(LKM_init);
module_exit(LKM_exit);

