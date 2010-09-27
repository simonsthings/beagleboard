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
	{ 0x82e5cdf6, "platform_device_put" },
	{ 0x86b4a265, "platform_device_add" },
	{ 0x9e883ca3, "platform_device_add_data" },
	{ 0xb0ca058f, "platform_device_add_resources" },
	{ 0x8a32c40a, "platform_device_alloc" },
	{ 0xd200878a, "platform_device_unregister" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "76AFD0F3BAB6CAACB891B8D");
