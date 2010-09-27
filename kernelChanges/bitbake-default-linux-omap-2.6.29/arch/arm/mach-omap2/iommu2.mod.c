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
	{ 0xd3c8668a, "install_iommu_arch" },
	{ 0x7d11c268, "jiffies" },
	{ 0x3bd1b1f6, "msecs_to_jiffies" },
	{ 0x8728140, "malloc_sizes" },
	{ 0xdc74cc24, "kmem_cache_alloc" },
	{ 0xea147363, "printk" },
	{ 0x5ec92c8f, "dev_driver_string" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0x51b1088c, "uninstall_iommu_arch" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=iommu";


MODULE_INFO(srcversion, "CF17DEA7B2A191794ECE66E");
