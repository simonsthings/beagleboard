#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
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
	{ 0xbed55ab0, "struct_module" },
	{ 0xfbdf07c0, "kmem_cache_destroy" },
	{ 0xadf42bd5, "__request_region" },
	{ 0xfbae1311, "clk_enable" },
	{ 0x788fe103, "iomem_resource" },
	{ 0x8728140, "malloc_sizes" },
	{ 0x32a0e4d8, "clk_disable" },
	{ 0xbed60566, "sub_preempt_count" },
	{ 0x4bcfcd3, "clk_put" },
	{ 0x5b20b95e, "mutex_unlock" },
	{ 0xe2d5255a, "strcmp" },
	{ 0xfa2a45e, "__memzero" },
	{ 0xfbd33a72, "__mutex_init" },
	{ 0xea147363, "printk" },
	{ 0xc917e655, "debug_smp_processor_id" },
	{ 0xdfb01a80, "cpu_v7_dcache_clean_area" },
	{ 0x36ebe42d, "kmem_cache_free" },
	{ 0x253659dc, "mutex_lock" },
	{ 0x4c6ff041, "add_preempt_count" },
	{ 0x1450a64f, "platform_get_resource" },
	{ 0x573ce53e, "platform_driver_register" },
	{ 0x43b0c9c3, "preempt_schedule" },
	{ 0xec3c1144, "module_put" },
	{ 0x247788b4, "driver_find_device" },
	{ 0xdc74cc24, "kmem_cache_alloc" },
	{ 0x93fca811, "__get_free_pages" },
	{ 0x1fc91fb2, "request_irq" },
	{ 0xe9ce8b95, "omap_ioremap" },
	{ 0x15331242, "omap_iounmap" },
	{ 0x5ec92c8f, "dev_driver_string" },
	{ 0x9bce482f, "__release_region" },
	{ 0x59f49967, "clk_get" },
	{ 0xac5da09f, "kmem_cache_create" },
	{ 0x4302d0eb, "free_pages" },
	{ 0x37a0cba, "kfree" },
	{ 0x3c08c4d, "platform_get_irq" },
	{ 0x62b5ab63, "platform_driver_unregister" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "27757FB0B907431CF8EA0F6");
