#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x133d294f, "module_layout" },
	{ 0xc6e7b43c, "kthread_stop" },
	{ 0x602182e, "class_destroy" },
	{ 0xfa83b84e, "device_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x37a0cba, "kfree" },
	{ 0x1492003b, "cdev_del" },
	{ 0xe321f4fa, "wake_up_process" },
	{ 0x2b32fff4, "kthread_create_on_node" },
	{ 0xa2ae57e3, "device_create" },
	{ 0x441cbcdc, "__class_create" },
	{ 0xdb884948, "cdev_add" },
	{ 0x3c327871, "cdev_init" },
	{ 0x5f4228d6, "kmem_cache_alloc_trace" },
	{ 0xa773c546, "kmalloc_caches" },
	{ 0x3fd78f3b, "register_chrdev_region" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x8ddd8aad, "schedule_timeout" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0xdb05c2fd, "current_task" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x92997ed8, "_printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "F7A9805AB8F47D0EE4FC9EF");
