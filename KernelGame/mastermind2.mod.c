#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x27d38ece, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x9749cf6, __VMLINUX_SYMBOL_STR(platform_device_unregister) },
	{ 0x8027217d, __VMLINUX_SYMBOL_STR(platform_driver_unregister) },
	{ 0x8f078685, __VMLINUX_SYMBOL_STR(__platform_driver_register) },
	{ 0x5257c34a, __VMLINUX_SYMBOL_STR(platform_device_register_full) },
	{ 0xad6b1891, __VMLINUX_SYMBOL_STR(file_ns_capable) },
	{ 0xe7f630, __VMLINUX_SYMBOL_STR(init_user_ns) },
	{ 0xb44ad4b3, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0x6dbdde60, __VMLINUX_SYMBOL_STR(remap_pfn_range) },
	{ 0x3744cf36, __VMLINUX_SYMBOL_STR(vmalloc_to_pfn) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x362ef408, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x8946e57f, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0xd6ee688f, __VMLINUX_SYMBOL_STR(vmalloc) },
	{ 0x3b341b5b, __VMLINUX_SYMBOL_STR(kmem_cache_alloc) },
	{ 0xdc06f3cb, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xf98f620c, __VMLINUX_SYMBOL_STR(cs421net_enable) },
	{ 0x976d2ff5, __VMLINUX_SYMBOL_STR(device_create_file) },
	{ 0xd6b8e852, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0xc8964a02, __VMLINUX_SYMBOL_STR(misc_register) },
	{ 0xcd88ce64, __VMLINUX_SYMBOL_STR(cs421net_get_data) },
	{ 0x754d539c, __VMLINUX_SYMBOL_STR(strlen) },
	{ 0x6c1988ba, __VMLINUX_SYMBOL_STR(_raw_spin_unlock) },
	{ 0x4ca9669f, __VMLINUX_SYMBOL_STR(scnprintf) },
	{ 0xdbbee5cd, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0xc1514a3b, __VMLINUX_SYMBOL_STR(free_irq) },
	{ 0xe7051e03, __VMLINUX_SYMBOL_STR(cs421net_disable) },
	{ 0xd8cd6060, __VMLINUX_SYMBOL_STR(device_remove_file) },
	{ 0xe8deebe3, __VMLINUX_SYMBOL_STR(misc_deregister) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x999e8297, __VMLINUX_SYMBOL_STR(vfree) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=xt_cs421net";


MODULE_INFO(srcversion, "04FFF23896BB926EF55E46B");
