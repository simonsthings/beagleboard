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
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x256887e1, "warn_slowpath" },
	{ 0xca950f86, "mem_map" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x8728140, "malloc_sizes" },
	{ 0xf7802486, "__aeabi_uidivmod" },
	{ 0xcaf2d853, "ioremap_page" },
	{ 0xe4c80097, "cacheid" },
	{ 0xb5f2d2c, "sg_next" },
	{ 0x5b20b95e, "mutex_unlock" },
	{ 0x999e8297, "vfree" },
	{ 0xe707d823, "__aeabi_uidiv" },
	{ 0xfa2a45e, "__memzero" },
	{ 0xea147363, "printk" },
	{ 0xdfb01a80, "cpu_v7_dcache_clean_area" },
	{ 0x36ebe42d, "kmem_cache_free" },
	{ 0x253659dc, "mutex_lock" },
	{ 0xe0a71f22, "iopgtable_store_entry" },
	{ 0xc440c451, "sg_alloc_table" },
	{ 0xdc74cc24, "kmem_cache_alloc" },
	{ 0x8a7d1c31, "high_memory" },
	{ 0xe9ce8b95, "omap_ioremap" },
	{ 0x15331242, "omap_iounmap" },
	{ 0x5ec92c8f, "dev_driver_string" },
	{ 0xac5da09f, "kmem_cache_create" },
	{ 0xda1fc224, "cpu_cache" },
	{ 0x5a323465, "__get_vm_area" },
	{ 0x37a0cba, "kfree" },
	{ 0x94961283, "vunmap" },
	{ 0x45a55ec8, "__iounmap" },
	{ 0xb534763c, "sg_free_table" },
	{ 0x525ed110, "vmalloc_to_page" },
	{ 0x7fe239c3, "iopgtable_clear_entry" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=iommu";


MODULE_INFO(srcversion, "06577D1637DF5B73307C826");
