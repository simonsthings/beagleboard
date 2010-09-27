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
	{ 0x64ec254f, "module_layout" },
	{ 0x5f8d3981, "clk_enable" },
	{ 0xbaff5713, "clk_disable" },
	{ 0x2e1ca751, "clk_put" },
	{ 0x9c55acd9, "omap_mbox_unregister" },
	{ 0x7d11c268, "jiffies" },
	{ 0xea147363, "printk" },
	{ 0xa6ed74b2, "platform_get_resource" },
	{ 0x44fa1486, "platform_driver_register" },
	{ 0x78c19780, "omap_mbox_register" },
	{ 0x3bd1b1f6, "msecs_to_jiffies" },
	{ 0xe9ce8b95, "omap_ioremap" },
	{ 0x15331242, "omap_iounmap" },
	{ 0x1b1c4a9a, "dev_driver_string" },
	{ 0x73c1a69e, "clk_get" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x2bca3f1b, "platform_driver_unregister" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=mailbox";


MODULE_INFO(srcversion, "FE68512E25DA865FF62F602");
