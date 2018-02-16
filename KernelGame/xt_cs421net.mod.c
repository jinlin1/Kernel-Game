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
	{ 0xfeb12a2a, __VMLINUX_SYMBOL_STR(xt_unregister_target) },
	{ 0xdc06f3cb, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x43a53735, __VMLINUX_SYMBOL_STR(__alloc_workqueue_key) },
	{ 0xc70525df, __VMLINUX_SYMBOL_STR(trigger_irq) },
	{ 0x88bfa7e, __VMLINUX_SYMBOL_STR(cancel_work_sync) },
	{ 0x8d77aa40, __VMLINUX_SYMBOL_STR(complete_all) },
	{ 0xa292927d, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irqrestore) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xb2ff7ddd, __VMLINUX_SYMBOL_STR(wait_for_completion_interruptible) },
	{ 0x8c03d20c, __VMLINUX_SYMBOL_STR(destroy_workqueue) },
	{ 0x12e1ab8e, __VMLINUX_SYMBOL_STR(xt_register_target) },
	{ 0x3b341b5b, __VMLINUX_SYMBOL_STR(kmem_cache_alloc) },
	{ 0x23bbbca0, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irqsave) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x69acdf38, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0x2e0d2f7f, __VMLINUX_SYMBOL_STR(queue_work_on) },
	{ 0x96ce5809, __VMLINUX_SYMBOL_STR(complete) },
	{ 0x726b65ad, __VMLINUX_SYMBOL_STR(completion_done) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=x_tables";


MODULE_INFO(srcversion, "761B0C7C18691CA595BC89C");
